/*!
 *    @file  circuit.hpp
 *   @brief circuit super class, holding the linar system and components
 *
 *  <+DETAILED+>
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

#ifndef  system_INC
#define  system_INC

#include <vector>
#include <map>
#include <tuple>
#include <string>

#include <cusparse.h>
#include <cusolverSp.h>

#include "component.hpp"

namespace rtspice::circuit {

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
        float rtol    = 1e-3f;
        float atol    = 1e-4f;
        int   maxiter = 10000;
      } params_;

      struct {
        cusparseHandle_t   sparse_handle;
        cusolverSpHandle_t solver_handle;
      } context_;

      struct {
        std::map<std::string, std::size_t> names;
        std::map<std::pair<std::string,std::string>, float*> pointers;
      } nodes_;

      struct {
        std::size_t         m, nnz;       //problem size

        cuda_ptr_<int>      row, col;
        cuda_ptr_<float>    A, A_static, A_dynamic;

        cusparseMatDescr_t  desc_A;

        cuda_ptr_<float>    b, b_static, b_dynamic;
        cuda_ptr_<float>    x, state;
        cuda_ptr_<float>    x_prev;

        float               ground_A;
        const float         ground_x = 0;

        float time, delta_time;

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


    public:

      circuit(std::vector<components::component::ptr> components);
      ~circuit();

      int step_();       //basic step
      int nr_step_();    //iterate basic step until convergence
      int time_step_();  //nr_step_ then advance time

      //add node name to pool
      void register_node(const std::string& node_name);

      //add matrix entry to pool
      void register_entry(const std::pair<std::string,std::string>& entry);

      //recover address of matrix entry
      float* get_A(const std::pair<std::string,std::string>& entry);

      //recover address of matrix entry
      float* get_b(const std::string& node_name);

      //recover address of previous state entry
      const float* get_x(const std::string& node_name) const;

      //recover address of current solution entry
      const float* get_state(const std::string& node_name) const;

      const float* get_time() const;
      const float* get_delta_time() const;

      auto& nodes() const {
        return nodes_.names;
      }

      auto solution(const std::string& node_name) const {
        return system_.x[nodes_.names.at(node_name)];
      }
  };

}		// -----  end of namespace rtspice::circuit  -----


#endif   // ----- #ifndef circuit_INC  -----
