/*!
 *    @file  source.hpp
 *   @brief independent and controlled source definitions
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

#ifndef  source_INC
#define  source_INC

#include <string>

#include "component.hpp"

namespace rtspice::components {

  class voltage_source : public component {
    public:
      voltage_source(std::string id,
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

  class current_source : public component {
    public:
      current_source(std::string id,
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

#endif   // ----- #ifndef source_INC  ----- 
