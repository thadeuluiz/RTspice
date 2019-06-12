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
                  system_.row, system_.col, system_.A,
                  &system_.dA,
                  context_.queue);

    //the source vector
    magma_svset(system_.m, 1,
                system_.b, &system_.db,
                context_.queue);

    //and the solution vector
    magma_svset(system_.m, 1,
                system_.x, &system_.dx,
                context_.queue);

    //trick magma into thinking this is gpu memory
    system_.dA.memory_location = Magma_DEV;
    system_.db.memory_location = Magma_DEV;
    system_.dx.memory_location = Magma_DEV;


    auto& opts = context_.opts;

    opts.solver_par.solver     = Magma_CGS;
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
    magma_free(system_.row);
    magma_free(system_.col);

    cudaFree(system_.A);
    cudaFree(system_.b);
    cudaFree(system_.x);

    magma_free_cpu(system_.state);

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
    cudaMallocManaged(&system_.row, (m+1)*sizeof(magma_index_t));
    cudaMallocManaged(&system_.col, (nnz)*sizeof(magma_index_t));

    cudaMallocManaged(&system_.A, nnz*sizeof(float));
    cudaMallocManaged(&system_.b, m  *sizeof(float));
    cudaMallocManaged(&system_.x, m  *sizeof(float));

    magma_smalloc_cpu(&system_.state, m);

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
        coordinate_it->second = &system_.A[col_offset + row_entry];

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
    fill_n(system_.A, nnz,  0.0f);
    fill_n(system_.b, m,    0.0f);

    //stamp the system
    for(auto&& c: components_) c->fill(0,0);

    auto& opts = context_.opts;

    magma_s_solver(system_.dA, system_.db, &system_.dx, &opts, context_.queue );
    //magma_scgs(system_.dA, system_.db, &system_.dx, &opts.solver_par, context_.queue);

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
    return &system_.b[nodes_.names.at(e)];
  }

  const float* circuit::get_x(const string& n) const {
    if(n == "0")
      return &system_.ground_state;
    return &system_.x[nodes_.names.at(n)];
  }

  const float* circuit::get_state(const string& n) const {
    if(n == "0")
      return &system_.ground_state;
    return &system_.state[nodes_.names.at(n)];
  }

} // -----  end of namespace rtspice::circuit  -----
