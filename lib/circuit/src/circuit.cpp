/*!
 *    @file  circuit.cpp
 *   @brief circuit holder and simulation implementation
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/09/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include <cassert>

#include <algorithm>

#include <cuda_runtime.h>

#include "circuit.hpp"

using std::vector;
using std::move;
using std::pair;
using std::string;
using std::fill_n;

namespace rtspice::circuit {

  circuit::circuit(vector<components::component::ptr> components) :
    components_{move(components)} {

    //initialize context
    assert((magma_init() == MAGMA_SUCCESS) && "error initializing magma");
    magma_queue_create(0, &context_.queue);
    assert((context_.queue != NULL) && "error creating magma queue");

    //collect all nodes and entries
    for(auto& c: components_) c->register_nodes(*this);

    //prepare matrices
    setup_nodes_();

    //feed the system pointers to the components
    for(auto& c: components_) c->setup_entries(*this);

    //prepare host and device matrices
    magma_scsrset(system_.m, system_.m,
                  system_.row, system_.col, system_.val,
                  &system_.A,
                  context_.queue);

    magma_s_mtransfer(system_.A, &system_.dA,
                      Magma_CPU, Magma_DEV,
                      context_.queue);

    //the source vector
    magma_svset(system_.m, 1,
                system_.sources, &system_.b,
                context_.queue);

    magma_s_mtransfer(system_.b, &system_.db,
                      Magma_CPU, Magma_DEV,
                      context_.queue);

    //and the solution vector
    magma_svset(system_.m, 1,
                system_.sol, &system_.x_,
                context_.queue);

    magma_s_mtransfer(system_.x_, &system_.dx_,
                      Magma_CPU, Magma_DEV,
                      context_.queue);

    assert(system_.A.val  == system_.val);
    assert(system_.b.val  == system_.sources);
    assert(system_.x_.val == system_.sol);

    assert(system_.dA.val  != nullptr);
    assert(system_.db.val  != nullptr);
    assert(system_.dx_.val != nullptr);

    auto& opts = context_.opts;

    opts.solver_par.solver     = Magma_GMRES;
    opts.solver_par.restart    = 30;
    opts.solver_par.maxiter    = 1000;
    opts.solver_par.rtol       = 1e-7;
    //opts.precond_par.solver    = Magma_ILU;
    //opts.precond_par.levels    = 0;
    //opts.precond_par.trisolver = Magma_CUSOLVE;

    magma_ssolverinfo_init(&opts.solver_par, &opts.precond_par, context_.queue);

  }

  circuit::~circuit() {

    const auto& queue = context_.queue;
    //release opts
    magma_ssolverinfo_free(&context_.opts.solver_par,
                           &context_.opts.precond_par,
                           queue);

    //release host buffers
    magma_free_pinned(system_.row);
    magma_free_pinned(system_.col);
    magma_free_pinned(system_.val);
    magma_free_pinned(system_.sources);

    magma_free_pinned(system_.sol);
    magma_free_pinned(system_.state);

    //release host matrices
    magma_smfree(&system_.A, queue);
    magma_smfree(&system_.b, queue);
    magma_smfree(&system_.x_,queue);

    //release device matrices
    magma_smfree(&system_.dA, queue);
    magma_smfree(&system_.db, queue);
    magma_smfree(&system_.dx_,queue);

    magma_queue_destroy(queue);
    magma_finalize();

  }

  void circuit::setup_nodes_() {

    //fill node_name -> index map
    //nodes are sorted lexicographically
    std::for_each(
        nodes_.names.begin(),
        nodes_.names.end(),
        [i = 0](auto&& p) mutable { p.second = i++; });

    const auto m   = nodes_.names.size();
    const auto nnz = nodes_.address.size();

    system_.m = m;
    system_.nnz = nnz;

    //allocate index and value buffers
    magma_index_malloc_pinned(&system_.row, m+1);
    magma_index_malloc_pinned(&system_.col, nnz);
    magma_smalloc_pinned(&system_.val, nnz);
    magma_smalloc_pinned(&system_.sources, m);

    magma_smalloc_pinned(&system_.sol, m);
    magma_smalloc_pinned(&system_.state, m);

    auto name_it = nodes_.names.cbegin();
    auto coordinate_it = nodes_.address.begin();

    for(auto row = 0; row < m; ++row, ++name_it) {

      //node name of current row
      auto&& row_name = name_it->first;

      auto upper_bound = std::upper_bound(
          coordinate_it,
          nodes_.address.end(),
          row_name,
          [](auto&& rname, auto&& kv){ return rname < kv.first.first; });

      //current row entries are in [coordinate_it, upper_bound)

      magma_index_t col_offset = system_.row[row]; //offset onto col vector
      magma_index_t row_entry  = 0;

      for(; coordinate_it != upper_bound; ++coordinate_it, ++row_entry) {

        const auto& col_name = coordinate_it->first.second;
        const auto col = nodes_.names.at(col_name);

        system_.col[col_offset + row_entry] = col;
        coordinate_it->second = &system_.val[col_offset + row_entry];

      }

      //advance row
      system_.row[row+1] = system_.row[row] + row_entry;
    }

    assert(system_.row[m] == nnz && "row filling failure");
    assert(coordinate_it == nodes_.address.end() && "not all coordinates used");

  }

  magma_int_t circuit::step_() {

    const auto nnz = system_.nnz;
    const auto m   = system_.m;

    //clear the buffers
    fill_n(system_.val    , nnz,  0.0f);
    fill_n(system_.sources, m,    0.0f);

    //stamp the system
    for(auto&& c: components_) c->fill(0,0);

    //pass new data to gpu
    magma_ssetvector_async(nnz,
                           system_.A.val, 1,
                           system_.dA.dval, 1,
                           context_.queue);

    magma_ssetvector_async(m,
                           system_.b.val, 1,
                           system_.db.dval, 1,
                           context_.queue);

    auto& opts = context_.opts;

    magma_s_solver(system_.dA, system_.db, &system_.dx_, &opts, context_.queue );

    magma_sgetvector(m,
                     system_.dx_.dval, 1,
                     system_.x_.val, 1,
                     context_.queue);

    const auto numiter = opts.solver_par.numiter;

    return numiter;

  }

  void circuit::register_node(const string& n) {
    if(n != "0") //skip ground node
      nodes_.names.emplace(n, 0);
  }

  void circuit::register_entry(const pair<string, string>& e) {
    if(e.first != "0" && e.second != "0") //skip ground node entries
      nodes_.address.emplace(e, nullptr);
  }

  float* circuit::get_A(const pair<string, string>& e) {
    if(e.first == "0" || e.second == "0")
      return &system_.ground_entry;
    return nodes_.address.at(e);
  }

  float* circuit::get_b(const string& e) {
    if(e == "0")
      return &system_.ground_entry;
    return &system_.sources[nodes_.names.at(e)];
  }

  const float* circuit::get_x(const string& n) const {
    if(n == "0")
      return &system_.ground_state;
    return &system_.state[nodes_.names.at(n)];
  }

  const float* circuit::get_x_(const string& n) const {
    if(n == "0")
      return &system_.ground_state;
    return &system_.sol[nodes_.names.at(n)];
  }

} // -----  end of namespace rtspice::circuit  -----
