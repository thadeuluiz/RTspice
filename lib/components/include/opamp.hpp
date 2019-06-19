/*!
 *    @file  opamp.hpp
 *   @brief  ideal opamp definitions
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/13/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#ifndef  opamp_INC
#define  opamp_INC

#include "component.hpp"
#include "circuit.hpp"

namespace rtspice::components {

  /*!
   * @brief ideal opamp stamp
   *
   * output nodes: na, nb; control nodes nc, nd.
   * This model assumes linear stable operation, such that VC == VD
   *
   */
  class ideal_opamp : public component {
    public:
      ideal_opamp(std::string id,
                  std::string na,
                  std::string nb,
                  std::string nc,
                  std::string nd) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        nj_{ "@J" + id_ } {}

      virtual bool is_static()    const override { return true; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return false; }

      virtual void register_(circuit::circuit& c) override {

        c.register_node(na_);
        c.register_node(nb_);
        c.register_node(nc_);
        c.register_node(nd_);
        c.register_node(nj_);

        c.register_entry({na_, nj_});
        c.register_entry({nb_, nj_});
        c.register_entry({nj_, nc_});
        c.register_entry({nj_, nd_});

      }

      virtual void setup(circuit::circuit& c) override {

        Aaj_  = c.get_A({na_, nj_});
        Abj_  = c.get_A({nb_, nj_});
        Ajc_  = c.get_A({nj_, nc_});
        Ajd_  = c.get_A({nj_, nd_});

      }

      virtual void fill() const noexcept override {
        *Aaj_ += 1.0;
        *Abj_ -= 1.0;
        *Ajc_ += 1.0;
        *Ajd_ -= 1.0;
      }

    private:
      const std::string na_, nb_, nc_, nd_, nj_;
      circuit::entry_reference<float> Aaj_, Abj_, Ajc_, Ajd_;
  };

}		// -----  end of namespace rtspice::components  -----


#endif   // ----- #ifndef opamp_INC  -----
