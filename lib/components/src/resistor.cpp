/*!
 *    @file  resistor.cpp
 *   @brief  
 *
 *  <+DETAILED+>
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/10/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include "resistor.hpp"
#include "circuit.hpp"

namespace rtspice::components {

  linear_resistor::linear_resistor(std::string id,
                                   std::string np,
                                   std::string nm,
                                   double val) : 
        component{std::move(id)},
        a_{std::move(np)},
        b_{std::move(nm)},
        R_{val},
        G_{1/R_} { }

  void linear_resistor::register_nodes(circuit::circuit& circuit) {
    circuit.register_node(a_);
    circuit.register_node(b_);

    circuit.register_entry({a_, a_});
    circuit.register_entry({a_, b_});
    circuit.register_entry({b_, a_});
    circuit.register_entry({b_, b_});

  }

  void linear_resistor::setup_entries(circuit::circuit& circuit) {

    Aaa_ = circuit.get_A({a_, a_});
    Aab_ = circuit.get_A({a_, b_});
    Aba_ = circuit.get_A({b_, a_});
    Abb_ = circuit.get_A({b_, b_});

  }

  void linear_resistor::fill(double, double) {
    *Aaa_ += G_;
    *Aab_ -= G_;
    *Aba_ -= G_;
    *Abb_ += G_;
  }



}		// -----  end of namespace rtspice::components  ----- 
