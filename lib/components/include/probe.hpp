/*!
 *    @file  probe.hpp
 *   @brief  meta component to enable probing of variable
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/22/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */


#ifndef  probe_INC
#define  probe_INC

#include "component.hpp"

#include "circuit.hpp"


namespace rtspice::components {

  /*!
   *  @brief  basic output probe: add this in the netlist to record variable name
   */
  class probe : public component {
    public:
      probe(std::string probe) :
        component{"PROBE:" + probe},
        probe_{std::move(probe)} {}

      virtual void register_(circuit::circuit& circuit) override {
        circuit.register_node(probe_);
      }

      virtual void setup(circuit::circuit& circuit) override {
        circuit.get_output(probe_) = circuit.get_x(probe_);
      }

      virtual void fill() const noexcept override {}


      virtual bool is_static()    const override { return true; }
      virtual bool is_dynamic()   const override { return false; }

      virtual bool is_nonlinear() const override { return false; }

    private:
      const std::string probe_;
  }; // -----  end of class probe  -----

}		// -----  end of namespace rtspice::components  -----


#endif   // ----- #ifndef probe_INC  -----
