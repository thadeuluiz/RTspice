/*!
 *    @file  source.hpp
 *   @brief independent and controlled source definitions
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
                     double val);

      virtual void register_nodes(circuit::circuit& c) override;
      virtual void setup_entries(circuit::circuit& c) override;
      virtual void fill(double t, double timestep) override;

    private:
      std::string a_, b_;
      std::string j_;
      double V_;

      float *Aaj_, *Abj_, *Aja_, *Ajb_, *bj_;
  };

  class current_source : public component {
    public:
      current_source(std::string id,
                     std::string np,
                     std::string nm,
                     double val);

      virtual void register_nodes(circuit::circuit& c) override;
      virtual void setup_entries(circuit::circuit& c) override;
      virtual void fill(double t, double timestep) override;

    private:
      std::string a_, b_;
      double I_;
      float *ba_, *bb_;

  };

}		// -----  end of namespace rtspice::components  ----- 

#endif   // ----- #ifndef source_INC  ----- 
