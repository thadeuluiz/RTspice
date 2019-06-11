/*!
 *    @file  source.cpp
 *   @brief  Voltage and current sources implementations
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/11/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */



#include "source.hpp"
#include "circuit.hpp"

using namespace std::string_literals;

using std::string;
using std::move;


namespace rtspice::components {

  voltage_source::voltage_source(string id,
                                 string np,
                                 string nm,
                                 double val) :
    component{ move(id) },
    a_{ move(np) },
    b_{ move(nm) },
    j_{ "J"s + id_ },
    V_{val} {}

  void voltage_source::register_nodes(circuit::circuit &c) {

    c.register_node(a_);
    c.register_node(b_);
    c.register_node(j_);

    c.register_entry({a_, j_});
    c.register_entry({b_, j_});
    c.register_entry({j_, a_});
    c.register_entry({j_, b_});
  }

  void voltage_source::setup_entries(circuit::circuit &c) {

    Aaj_ = c.get_A({a_, j_});
    Abj_ = c.get_A({b_, j_});
    Aja_ = c.get_A({j_, a_});
    Ajb_ = c.get_A({j_, b_});
    bj_  = c.get_b(j_);

  }

  void voltage_source::fill(double, double) {
    *Aaj_ += 1;
    *Abj_ -= 1;
    *Aja_ -= 1;
    *Ajb_ += 1;
    *bj_  -= V_;
  }

  current_source::current_source(string id,
                                 string np,
                                 string nm,
                                 double val) :
        component{ move(id) },
        a_{ move(np) },
        b_{ move(nm) },
        I_{val} {}

  void current_source::register_nodes(circuit::circuit &c) {
    c.register_node(a_);
    c.register_node(b_);
  }

  void current_source::setup_entries(circuit::circuit &c) {
    ba_  = c.get_b(a_);
    bb_  = c.get_b(b_);
  }

  void current_source::fill(double, double) {
    *ba_ -= I_;
    *bb_ += I_;
  }


}		// -----  end of namespace rtspice::components  -----
