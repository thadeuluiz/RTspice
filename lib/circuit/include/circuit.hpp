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
#include <tuple>
#include <string>

#include <magma_v2.h>
#include <magmasparse.h>

#include "component.hpp"

namespace rtspice::circuit {

  class circuit {
    private:
      std::vector<components::component::ptr> components_;

      struct {
        magma_queue_t queue;
        magma_sopts opts;
      } context_;

      struct {
        //mapping of node name to index
        std::map<std::string, magma_index_t> names;
        //mapping of non-zero entry to buffer address
        std::map<std::pair<std::string,std::string>, float*> address;
      } nodes_;

      struct {
        //problem size
        magma_int_t m, nnz;
        //CPU side
        magma_index_t              *row, *col;  //indices for the sparse matrix
        float                      *A;          //the sparse matrix entries
        float                      *b;          //the source vector
        float                      *x, *state;  //current state and next state

        magma_s_matrix             dA{Magma_CSR}, db{Magma_CSR}, dx{Magma_CSR};

        float                      ground_entry; //dummy address for stamping
                                                  //of ground nodes

        const float                ground_state = 0; //dummy address for
                                                      //reading of ground node
                                                      //state
      } system_;

      void setup_nodes_();


    public:

      circuit(std::vector<components::component::ptr> components);
      ~circuit();

      magma_int_t step_();

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

      auto& nodes() const {
        return nodes_.names;
      }

      auto solution(const std::string& node_name) const {
        return system_.x[nodes_.names.at(node_name)];
      }
  };

}		// -----  end of namespace rtspice::circuit  -----


#endif   // ----- #ifndef circuit_INC  -----
