/*!
 *    @file  circuit.cpp
 *   @brief circuit holder and simulation implementation
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/09/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include "circuit.hpp"

#include <cstdint>
#include <algorithm>
#include <execution>
#include <numeric>

using namespace std;

constexpr auto parallel_tag = execution::unsequenced_policy{};

namespace rtspice::circuit {

  using components::component;

  circuit::circuit(vector<component::ptr> components) {

      setup_context_();              //init cuda
      setup_components_(components); //get component classes

      register_nodes_();             //get needed variables
      setup_system_();               //allocate linear system memory
      setup_nodes_();                //feed system pointers to components

      init_components_();

      setup_static_();               //feed static stamps
  }

  circuit::~circuit() {
    teardown_system_();
    teardown_context_();
  }

  void circuit::setup_components_(const vector<component::ptr>& comps) {
    //split components into classes
    copy_if(comps.begin(), comps.end(),
        back_inserter(components_.static_),
        [](auto&& c) { return c->is_static(); });

    copy_if(comps.begin(), comps.end(),
        back_inserter(components_.dynamic),
        [](auto&& c) { return c->is_dynamic(); });

    copy_if(comps.begin(), comps.end(),
        back_inserter(components_.nonlinear),
        [](auto&& c) { return c->is_nonlinear(); });
  }

  void circuit::setup_context_() {
    //initialize context
    int status;
    status = cusparseCreate(&context_.sparse_handle);
    assert(status == CUSPARSE_STATUS_SUCCESS && "cuSolver initialization failure");

    status = cusolverSpCreate(&context_.solver_handle);
    assert(status == CUSOLVER_STATUS_SUCCESS && "cuSolver initialization failure");
  }

  void circuit::register_nodes_() {

    for(auto&& c: components_.static_)   c->register_(*this);
    for(auto&& c: components_.dynamic)   c->register_(*this);
    for(auto&& c: components_.nonlinear) c->register_(*this);

  }

  void circuit::setup_system_() {

    const auto m   = nodes_.names.size();
    const auto nnz = nodes_.pointers.size();

    auto& sys = system_;

    sys.m   = m;
    sys.nnz = nnz;

    //allocate index and value buffers
    sys.row = cuda_malloc_<int>(m+1);
    sys.col = cuda_malloc_<int>(nnz);

    sys.A_static    = cuda_malloc_<float>(nnz);
    sys.A_dynamic   = cuda_malloc_<float>(nnz);
    sys.A_nonlinear = cuda_malloc_<float>(nnz);

    sys.b_static    = cuda_malloc_<float>(m);
    sys.b_dynamic   = cuda_malloc_<float>(m);
    sys.b_nonlinear = cuda_malloc_<float>(m);

    sys.states[0] = cuda_malloc_<float>(m);
    sys.states[1] = cuda_malloc_<float>(m);
    sys.states[2] = cuda_malloc_<float>(m);

    sys.A = sys.A_static.get();
    sys.b = sys.b_static.get();

    sys.x       = sys.states[0].get();
    sys.xn      = sys.states[1].get();
    sys.x_state = sys.states[2].get();
    //matrix descriptor
    cusparseCreateMatDescr(&sys.desc_A);
    cusparseSetMatType(sys.desc_A,      CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(sys.desc_A, CUSPARSE_INDEX_BASE_ZERO);

  }

  void circuit::teardown_system_() {
    cusparseDestroyMatDescr(system_.desc_A);
  }

  void circuit::setup_nodes_() {

    auto& sys = system_;

    const auto m   = sys.m;
    const auto nnz = sys.nnz;

    auto& names = nodes_.names;

    //fill node_name -> index map
    for_each(begin(names), end(names),
                  [i = 0](auto&& p) mutable { p.second = i++; });

    auto row_begin = nodes_.pointers.begin();
    //prepare initial sparsity figure
    for(const auto& [row_name, row] : names) {

      auto row_end = upper_bound(
          row_begin,
          nodes_.pointers.end(),
          row_name,
          [](auto&& rname, auto&& kv){ return rname < kv.first.first; });

      //current row entries are in [row_begin, row_end) the list

      auto offset = sys.row[row]; //offset onto col/value buffers
      auto entry_no  = 0;

      for(; row_begin != row_end; ++row_begin, ++entry_no) {
        const auto& col_name = row_begin->first.second;
        const auto  col_idx  = nodes_.names.at(col_name);
        sys.col[offset + entry_no] = col_idx;
        row_begin->second = offset + entry_no;
      }

      //advance row
      sys.row[row+1] = sys.row[row] + entry_no;
    }
    assert(sys.row[m] == nnz && "row filling failure");
    assert(row_begin == nodes_.pointers.end() && "not all coordinates used");

    //get optimal pattern
    const auto perm = std::make_unique<int[]>(m);
    auto status = cusolverSpXcsrsymmdqHost(context_.solver_handle, m, nnz,
        sys.desc_A, sys.row.get(), sys.col.get(), perm.get());
    assert(status == CUSOLVER_STATUS_SUCCESS);

    //allocate permutation worksize
    size_t bsize;
    status = cusolverSpXcsrperm_bufferSizeHost(context_.solver_handle, m, m, nnz,
       sys.desc_A, sys.row.get(), sys.col.get(), perm.get(), perm.get(), &bsize);
    assert(status == CUSOLVER_STATUS_SUCCESS);

    const auto work = std::make_unique<uint8_t[]>(bsize);
    const auto map  = std::make_unique<int[]>(nnz);
    iota(map.get(), map.get()+nnz, 0);

    //perform Q * A * Q^T
    status = cusolverSpXcsrpermHost(context_.solver_handle, m, m, nnz,
       sys.desc_A, sys.row.get(), sys.col.get(),
       perm.get(), perm.get(), map.get(),
       work.get());
    assert(status == CUSOLVER_STATUS_SUCCESS);

    //update node name map
    for(auto&& [_, idx]: nodes_.names)
      idx = find(perm.get(), perm.get()+m, idx) - perm.get();

    for(auto&& kv: nodes_.pointers){

      const auto& [na, nb] = kv.first;
      const auto a = nodes_.names.at(na), b = nodes_.names.at(nb);

      const auto row = sys.row.get(),
                 col = sys.col.get();

      const auto ofs = find(&col[row[a]], &col[row[a+1]], b) - col;

      assert(ofs != row[a+1]);
      assert(map[ofs] == kv.second);

      kv.second = ofs;
    }

  }

  void circuit::teardown_context_() {

    int status;
    status = cusolverSpDestroy(context_.solver_handle);
    assert(status == CUSOLVER_STATUS_SUCCESS && "solver cleanup failure");
    status = cusparseDestroy(context_.sparse_handle);
    assert(status == CUSPARSE_STATUS_SUCCESS && "sparse cleanup failure");

  }

  void circuit::init_components_() {

    for(auto&& c: components_.static_)   c->setup(*this);
    for(auto&& c: components_.dynamic)   c->setup(*this);
    for(auto&& c: components_.nonlinear) c->setup(*this);

  }

  void circuit::setup_static_() {

    auto& sys = system_;
    const auto m = sys.m, nnz = sys.nnz;

    sys.A = sys.A_static.get();
    sys.b = sys.b_static.get();

    fill_n(sys.A, nnz, 0.0);
    fill_n(sys.b, m,   0.0);

    fill_n(sys.x,       m,   0.0);
    fill_n(sys.xn,      m,   0.0);
    fill_n(sys.x_state, m,   0.0);

    for(auto&& c: components_.static_) c->fill();

    copy_n(sys.A, nnz, sys.A_dynamic.get());
    copy_n(sys.A, nnz, sys.A_nonlinear.get());

    copy_n(sys.b, nnz, sys.b_dynamic.get());
    copy_n(sys.b, nnz, sys.b_nonlinear.get());

  }

  int circuit::advance_(float delta_t) {
    auto& sys = system_;

    //advance time
    sys.delta_time = delta_t;
    sys.time      += delta_t;

    const auto m = sys.m, nnz = sys.nnz;

    //prefill with static data

    //set the receiving pointers
    sys.A = sys.A_dynamic.get();
    sys.b = sys.b_dynamic.get();

    copy_n(parallel_tag, sys.A_static.get(), nnz, sys.A);
    copy_n(parallel_tag, sys.b_static.get(), m,   sys.b);

    //load dynamic data
    for(auto&& c: components_.dynamic) c->fill();

    //iterate until convergence
    const auto i = nr_step_();
    if(i < 0) return i;

    //store new t1
    copy_n(parallel_tag, sys.x, m, sys.x_state);

    return i;
  }

  int circuit::nr_step_() {

    auto& sys = system_;
    const auto m = sys.m, nnz = sys.nnz;

    const auto rtol    = params_.rtol;
    const auto atol    = params_.atol;
    const auto maxiter = params_.maxiter;

    //set the receiving pointers
    sys.A = sys.A_nonlinear.get();
    sys.b = sys.b_nonlinear.get();

    const auto close = [rtol, atol](float a, float b) {
      return abs(a-b) <= fma(rtol, abs(b), atol);
    };

    for(int i = 1; i <= maxiter; ++i) {

      //get pre-fill
      copy_n(parallel_tag, sys.A_dynamic.get(), nnz, sys.A);
      copy_n(parallel_tag, sys.b_dynamic.get(), m,   sys.b);

      //nonlinear fill
      for(auto&& c: components_.nonlinear) c->fill();

      //store old estimate in xn
      swap(sys.x, sys.xn);

      //update solution
      if(!solve_()) return -i;

      auto good = std::transform_reduce(parallel_tag,
                                        sys.x, sys.x + nnz,
                                        sys.xn,
                                        true,
                                        logical_and<bool>{},
                                        close);

      //all values converged
      if(good) return i;
    }

    //no convergence obtained
    return 0;

  }

  int circuit::solve_() {
    auto& sys = system_;

    int singular = 0;
    const auto status = cusolverSpScsrlsvluHost(context_.solver_handle,
        sys.m, sys.nnz, sys.desc_A,
        sys.A, sys.row.get(), sys.col.get(),
        sys.b,
        1e-16,
        0, //no reordering, leaks memory
        sys.x,
        &singular);

    assert(status == CUSOLVER_STATUS_SUCCESS);
    assert(singular == -1);

    return status == CUSOLVER_STATUS_SUCCESS && singular == -1;

  }

  void circuit::register_node(const string& n) {
    if(n != "0") //skip ground node
      nodes_.names.emplace(n, 0);
  }

  void circuit::register_entry(const pair<string, string>& e) {
    if(e.first != "0" && e.second != "0") //skip ground node entries
      nodes_.pointers.emplace(e, 0);
  }

  entry_reference<float> circuit::get_A(const pair<string, string>& ij) {
    if(ij.first == "0" || ij.second == "0"){
      return {&system_.ground_A, 0};
    }
    return {&system_.A, nodes_.pointers.at(ij)};
  }

  entry_reference<float> circuit::get_b(const string& n) {
    if(n == "0")
      return {&system_.ground_A, 0};
    return {&system_.b, nodes_.names.at(n)};
  }

  entry_reference<const float> circuit::get_x(const string& n) const {
    if(n == "0")
      return {&system_.ground_x, 0};
    return {&system_.x, nodes_.names.at(n)};
  }

  entry_reference<const float> circuit::get_state(const string& n) const {
    if(n == "0")
      return {&system_.ground_x, 0};
    return {&system_.x_state, nodes_.names.at(n)};
  }

  const float* circuit::get_time() const {
    return &system_.time;
  }

  const float* circuit::get_delta_time() const {
    return &system_.delta_time;
  }

} // -----  end of namespace rtspice::circuit  -----
