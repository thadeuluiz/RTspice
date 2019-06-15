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

#include <cassert>
#include <algorithm>
#include <numeric>

#include <cuda_runtime.h>

#include "circuit.hpp"

using std::vector;
using std::string;
using std::move;
using std::pair;

using std::fill_n;
using std::copy_n;
using std::copy_if;

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


      printf("created circuit with %zu nodes\n", nodes_.names.size());
      printf("nnz A = %zu\n", system_.nnz);

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

    system_.m   = m;
    system_.nnz = nnz;

    //allocate index and value buffers
    system_.row = cuda_malloc_<int>(m+1);
    system_.col = cuda_malloc_<int>(nnz);

    system_.A_static  = cuda_malloc_<float>(nnz);
    system_.A_dynamic = cuda_malloc_<float>(nnz);
    system_.A         = cuda_malloc_<float>(nnz);

    system_.b_static  = cuda_malloc_<float>(m);
    system_.b_dynamic = cuda_malloc_<float>(m);
    system_.b         = cuda_malloc_<float>(m);

    system_.state  = cuda_malloc_<float>(m);
    system_.x      = cuda_malloc_<float>(m);
    system_.x_prev = cuda_malloc_<float>(m);

    //matrix descriptor
    cusparseCreateMatDescr(&system_.desc_A);
    cusparseSetMatType(system_.desc_A,      CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(system_.desc_A, CUSPARSE_INDEX_BASE_ZERO);

  }

  void circuit::teardown_system_() {
    cusparseDestroyMatDescr(system_.desc_A);
  }

  void circuit::setup_nodes_() {

    //fill node_name -> index map
    //nodes are sorted lexicographically
    std::for_each(
        nodes_.names.begin(),
        nodes_.names.end(),
        [i = 0](auto&& p) mutable { p.second = i++; });

    const auto m   = system_.m;
    const auto nnz = system_.nnz;

    auto row_begin = nodes_.pointers.begin();

    for(auto& kv : nodes_.names) {

      //node name of current row
      const auto& row_name = kv.first;
      const auto  row      = kv.second;

      auto row_end = std::upper_bound(
          row_begin,
          nodes_.pointers.end(),
          row_name,
          [](auto&& rname, auto&& kv){ return rname < kv.first.first; });

      //current row entries are in [row_begin, row_end)
      auto offset = system_.row[row]; //offset onto col/value buffers
      auto entry_no  = 0;

      for(; row_begin != row_end; ++row_begin, ++entry_no) {
        const auto& col_name = row_begin->first.second;
        const auto  col_idx  = nodes_.names.at(col_name);

        system_.col[offset + entry_no] = col_idx;
        row_begin->second = &system_.A[offset + entry_no];
      }

      //advance row
      system_.row[row+1] = system_.row[row] + entry_no;
    }

    assert(system_.row[m] == nnz && "row filling failure");
    assert(row_begin == nodes_.pointers.end() && "not all coordinates used");

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

    fill_n(&system_.A[0], system_.nnz, 0.0);
    fill_n(&system_.b[0], system_.m, 0.0);
    fill_n(&system_.x[0], system_.m, 0.0);

    for(auto&& c: components_.static_) c->fill();

    copy_n(&system_.A[0], system_.nnz, &system_.A_static[0]);
    copy_n(&system_.A[0], system_.nnz, &system_.A_dynamic[0]); //for testing purposes

    copy_n(&system_.b[0], system_.m,   &system_.b_static[0]);
    copy_n(&system_.b[0], system_.m,   &system_.b_dynamic[0]);

  }

  int circuit::solve_() {

    auto& sys = system_;

    int singular = 0;
    const auto status = cusolverSpScsrlsvluHost(context_.solver_handle,
        sys.m, sys.nnz, sys.desc_A,
        sys.A.get(), sys.row.get(), sys.col.get(),
        sys.b.get(),
        1e-9,
        0, //no reordering, leaks memory
        sys.x.get(),
        &singular);

    assert(status == CUSOLVER_STATUS_SUCCESS);
    assert(singular == -1);

    return status == CUSOLVER_STATUS_SUCCESS && singular == -1;

  }

  int circuit::step_() {

    const auto nnz = system_.nnz;
    const auto m   = system_.m;

    const auto A_dyn = system_.A_dynamic.get();
    const auto b_dyn = system_.b_dynamic.get();

    auto A = system_.A.get();
    const auto row = system_.row.get();
    const auto col = system_.col.get();

    auto b = system_.b.get();
    auto x = system_.x.get();

    auto x_prev = system_.x_prev.get();

    //prefill buffers
    copy_n(A_dyn, nnz, A);
    copy_n(b_dyn, m, b);

    //stamp the system
    for(auto&& c: components_.nonlinear) c->fill();

    //hold previous solution
    copy_n(x, m, x_prev);

    return solve_();

  }

  int circuit::nr_step_() {

    const auto m   = system_.m;

    const auto rtol    = params_.rtol;
    const auto atol    = params_.atol;
    const auto maxiter = params_.maxiter;

    auto x = system_.x.get();
    auto x_prev = system_.x_prev.get();

    for(int i = 1; i <= maxiter; ++i) {

      //update solution
      if(!step_()) return -i;

      bool good = true;
      for(auto _ = 0; good && _ < m; ++_)
        good &= fabs(x[_] - x_prev[_]) < rtol*fabs(x_prev[_]) + atol;

      //all values converged
      if(good) return i;
    }

    //no convergence obtained
    return -maxiter-1;

  }

  int circuit::advance_(float delta_t) {
    //advance time
    system_.delta_time = delta_t;
    system_.time      += delta_t;

    const auto m = system_.m;
    const auto nnz = system_.nnz;

    //prefill with static data
    copy_n(&system_.A_static[0], nnz, &system_.A[0]);
    copy_n(&system_.b_static[0], m, &system_.b[0]);

    //load dynamic data
    for(auto&& c: components_.dynamic) c->fill();

    //store in dynamic buffers
    copy_n(&system_.A[0], nnz, &system_.A_dynamic[0]);
    copy_n(&system_.b[0], m, &system_.b_dynamic[0]);


    //iterate until convergence
    const auto i = nr_step_();
    if(i < 0) return i;

    //store new t0
    copy_n(&system_.x[0], m, &system_.state[0]);

    return i;
  }

  void circuit::register_node(const string& n) {
    if(n != "0") //skip ground node
      nodes_.names.emplace(n, 0);
  }

  void circuit::register_entry(const pair<string, string>& e) {
    if(e.first != "0" && e.second != "0") //skip ground node entries
      nodes_.pointers.emplace(e, nullptr);
  }

  float* circuit::get_A(const pair<string, string>& ij) {
    if(ij.first == "0" || ij.second == "0")
      return &system_.ground_A;
    return nodes_.pointers.at(ij);
  }

  float* circuit::get_b(const string& n) {
    if(n == "0")
      return &system_.ground_A;
    return &system_.b[nodes_.names.at(n)];
  }

  const float* circuit::get_x(const string& n) const {
    if(n == "0")
      return &system_.ground_x;
    return &system_.x[nodes_.names.at(n)];
  }

  const float* circuit::get_state(const string& n) const {
    if(n == "0")
      return &system_.ground_x;
    return &system_.state[nodes_.names.at(n)];
  }

  const float* circuit::get_time() const {
    return &system_.time;
  }

  const float* circuit::get_delta_time() const {
    return &system_.delta_time;
  }

} // -----  end of namespace rtspice::circuit  -----
