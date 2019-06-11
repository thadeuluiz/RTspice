/*!
 *    @file  resistor.hpp
 *   @brief resistor component definitions
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/08/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#ifndef  resistor_INC
#define  resistor_INC

#include <string>

#include "component.hpp"

namespace rtspice::components {

  class linear_resistor : public component {
    public:

      linear_resistor(std::string id,
                      std::string np,
                      std::string nm,
                      double val);

      virtual void register_nodes(circuit::circuit& c) override;
      virtual void setup_entries(circuit::circuit& c) override;
      virtual void fill(double t, double timestep) override;

    private:
      std::string a_, b_;
      double R_, G_;

      //required matrix references
      float *Aaa_, *Aab_, *Aba_, *Abb_;

  };


}		// -----  end of namespace rtspice::components  ----- 

#endif   // ----- #ifndef resistor_INC  ----- 
