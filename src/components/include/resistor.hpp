/*!
 *    @file  resistor.hpp
 *   @brief resistor component definitions
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/08/2019
 *      Revision:  none
 *      Compiler:  g++
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
                      double val) : 
        component{ std::move(id) },
        nplus_{std::move(np)},
        nminus_{std::move(nm)},
        value_{val} {}

    private:
      node nplus_;
      node nminus_;
      double value_;

  };


}		// -----  end of namespace rtspice::components  ----- 

#endif   // ----- #ifndef resistor_INC  ----- 
