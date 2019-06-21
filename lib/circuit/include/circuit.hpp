/*!
 *    @file  circuit.hpp
 *   @brief circuit super class, holding the linar system and components
 *
 *  <+DETAILED+>
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

#ifndef  system_INC
#define  system_INC

#include <vector>
#include <map>
#include <unordered_map>

#include <tuple>
#include <string>
#include <atomic>

#include <cassert>

#include <cuda_runtime.h>
#include <cusparse.h>
#include <cusolverSp.h>
#include <cusolverRf.h>

#include "component.hpp"

namespace rtspice::circuit {

  template<class T>
  class entry_reference {
    public:

      entry_reference(T* const* base = nullptr, std::ptrdiff_t offset = 0) :
        indirect_{base},
        offset_{offset} {}

      inline auto& operator*() const noexcept {
        return (*indirect_)[offset_];
      }

    private:
      T* const* indirect_;
      std::ptrdiff_t offset_;
  };

  class circuit {
    private:

      struct cuda_deleter_ {
        template<class T>
        void operator()(T* p){ cudaFreeHost(p); }
      };

      template<class T>
      using cuda_ptr_ = std::unique_ptr<T[], cuda_deleter_>;

      template<class T>
      static cuda_ptr_<T> cuda_malloc_(std::size_t sz) {

        T* ptr = nullptr;

        const auto res = cudaMallocHost(&ptr, sz*sizeof(T));
        assert(res == cudaSuccess);

        return cuda_ptr_<T>{ptr};
      }

      struct {
        std::vector<components::component::ptr> static_;
        std::vector<components::component::ptr> dynamic;
        std::vector<components::component::ptr> nonlinear;
      } components_;

      struct {
        float rtol    = 1e-3;
        float atol    = 1e-5;
        int   maxiter = 200;
      } params_;

      struct {
        cusparseHandle_t   sparse_handle;
        cusolverRfHandle_t refactor_handle;
        cusolverSpHandle_t solver_handle;
      } context_;

      struct {
        std::map<std::string, std::ptrdiff_t> names;
        std::map<std::pair<std::string,std::string>, std::ptrdiff_t> pointers;
      } nodes_;

      struct {

        //dynamic parameters (i.e. potentiometers, vgas, etc.)
        std::unordered_map<std::string, std::atomic<float>> params;

        //inputs
        std::unordered_map<std::string, float> inputs;

        std::size_t         m, nnz;       //problem size

        cuda_ptr_<int>      row, col;
        cuda_ptr_<float>    A_nonlinear, A_static, A_dynamic; //the buffers
        float*              A; //the output reference

        cusparseMatDescr_t  desc_A;

        cuda_ptr_<float>    b_nonlinear, b_static, b_dynamic;
        float               *b;
        cuda_ptr_<float>    states[3];

        float               *x, *xn, *x_state;

        float               dummy, zero = 0;
        float               *ground_A = &dummy;
        const float         *ground_x = &zero;

        float time = 0.0, delta_time;

      } system_;

      void setup_context_();
      void teardown_context_();

      void setup_components_(const std::vector<components::component::ptr>&);

      void register_nodes_();
      void setup_nodes_();

      void setup_system_();
      void teardown_system_();

      void init_components_();

      void setup_static_();

      int solve_();


    public:

      circuit(std::vector<components::component::ptr> components);
      ~circuit();

      int nr_step_();    //iterate basic step until convergence
      int advance_(float delta_t);  //nr_step_ then advance time

      //add node name to pool
      void register_node(const std::string& node_name);

      //add matrix entry to pool
      void register_entry(const std::pair<std::string,std::string>& entry);

      //recover address of matrix entry
      entry_reference<float> get_A(const std::pair<std::string,std::string>& entry);

      //recover address of matrix entry
      entry_reference<float> get_b(const std::string& node_name);

      //recover address of previous state entry
      entry_reference<const float> get_x(const std::string& node_name) const;

      //recover address of current solution entry
      entry_reference<const float> get_state(const std::string& node_name) const;

      auto& get_param(const std::string& param_name) {
        if(system_.params.find(param_name) == system_.params.end())
          system_.params[param_name] = 0.5f;
        return system_.params[param_name];
      };

      auto& get_input(const std::string& param_name) {
        return system_.inputs[param_name];
      };

      const float* get_time() const;
      const float* get_delta_time() const;

      auto& nodes() const { return nodes_.names; }
      auto& entries() const { return nodes_.pointers; }

      auto& params() { return system_.params; }
      auto& inputs() { return system_.inputs; }

  };

}		// -----  end of namespace rtspice::circuit  -----

#endif   // ----- #ifndef circuit_INC  -----
