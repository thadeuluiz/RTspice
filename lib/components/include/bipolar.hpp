/*!
 *    @file  bipolar.hpp
 *   @brief  basic bjt definitions
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/24/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#ifndef  bipolar_INC
#define  bipolar_INC

#include "resistor.hpp"
#include "sources.hpp"

namespace rtspice::components {

  class bipolar_npn : public component {
    public:
      virtual bool is_static()    const override { return false; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return true; }

      bipolar_npn(std::string id,
              std::string nc,
              std::string nb,
              std::string ne,
              float IS,
              float BF,
              float BR) :
        component{ std::move(id) },
        nc_{ std::move(nc) },
        nb_{ std::move(nb) },
        ne_{ std::move(ne) },
        nbe_{ "be@" + id_ },
        nbc_{ "bc@" + id_ },
        De_{ "De@" + id_, nbe_, ne_, IS, 1.0f },
        Dc_{ "Dc@" + id_, nbc_, nc_, IS, 1.0f },
        Fforward_{ "Ff@" + id_, nc_, nb_, nb_, nbe_, BF/(1.0f + BF) },
        Freverse_{ "Fr@" + id_, ne_, nb_, nb_, nbc_, BR/(1.0f + BR) } {}

      virtual void register_(circuit::circuit &c) override {
        De_.register_(c);
        Dc_.register_(c);
        Fforward_.register_(c);
        Freverse_.register_(c);
      }

      virtual void setup(circuit::circuit& c) override {
        De_.setup(c);
        Dc_.setup(c);
        Fforward_.setup(c);
        Freverse_.setup(c);
      }

      virtual void fill() const noexcept override {
        De_.fill();
        Dc_.fill();
        Fforward_.fill();
        Freverse_.fill();
      }

    private:
      const std::string nc_, nb_, ne_;
      const std::string nbe_, nbc_; //internal nodes
      basic_diode De_, Dc_;
      linear_cccs Fforward_, Freverse_;
  };

  class bipolar_pnp : public component {
    public:
      virtual bool is_static()    const override { return false; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return true; }

      bipolar_pnp(std::string id,
              std::string nc,
              std::string nb,
              std::string ne,
              float IS,
              float BF,
              float BR) :
        component{ std::move(id) },
        nc_{ std::move(nc) },
        nb_{ std::move(nb) },
        ne_{ std::move(ne) },
        nbe_{ "be@" + id_ },
        nbc_{ "bc@" + id_ },
        De_{ "De@" + id_, ne_, nbe_, IS, 1.0f },
        Dc_{ "Dc@" + id_, nc_, nbc_, IS, 1.0f },
        Fforward_{ "Ff@" + id_, nc_, nb_, nb_, nbe_, BF/(1.0f + BF) },
        Freverse_{ "Fr@" + id_, ne_, nb_, nb_, nbc_, BR/(1.0f + BR) } {}

      virtual void register_(circuit::circuit &c) override {
        De_.register_(c);
        Dc_.register_(c);
        Fforward_.register_(c);
        Freverse_.register_(c);
      }

      virtual void setup(circuit::circuit& c) override {
        De_.setup(c);
        Dc_.setup(c);
        Fforward_.setup(c);
        Freverse_.setup(c);
      }

      virtual void fill() const noexcept override {
        De_.fill();
        Dc_.fill();
        Fforward_.fill();
        Freverse_.fill();
      }

    private:
      const std::string nc_, nb_, ne_;
      const std::string nbe_, nbc_;
      basic_diode De_, Dc_;
      linear_cccs Fforward_, Freverse_;
  };

}		// -----  end of namespace rtspice::components  -----

#endif   // ----- #ifndef bipolar_INC  -----
