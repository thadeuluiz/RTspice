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

#include "circuit.hpp"
#include "component.hpp"

namespace rtspice::components {

  /*!
   * @brief generalized resistance template
   *
   * takes a class F, representing the transfer function j(v_ab). F must have an
   * operator() accepting the tension across the device, and must return
   * j(v_ab) and j'(v_ab) in a type that is compatible with structured bindings.
   * The static and nonlinear properties are passed through static constants
   * of F named 'static', 'dynamic', and 'nonlinear'
   *
   */
  template<class F>
  class resistor : public component {
    public:
      template<class... Args>
      resistor(std::string id,
               std::string na,
               std::string nb,
               Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        f_(std::forward<Args>(args)...) {}

      virtual bool is_static()    const override { return F::static_v; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return F::nonlinear_v; }

      virtual void register_(circuit::circuit& c) override {

        c.register_node(na_);
        c.register_node(nb_);

        c.register_entry({na_, na_});
        c.register_entry({na_, nb_});
        c.register_entry({nb_, na_});
        c.register_entry({nb_, nb_});

      }

      virtual void setup(circuit::circuit& c) override {

        Aaa_ = c.get_A({na_, na_});
        Aab_ = c.get_A({na_, nb_});
        Aba_ = c.get_A({nb_, na_});
        Abb_ = c.get_A({nb_, nb_});

        ba_  = c.get_b(na_);
        bb_  = c.get_b(nb_);

        xa_  = c.get_x(na_);
        xb_  = c.get_x(nb_);
      }

      virtual void fill() const noexcept override {

        const auto v = *xa_ - *xb_;
        const auto [f, df] = f_(v);

        const auto G = df;
        const auto I = f - G*v;

        *Aaa_ += G;
        *Aab_ -= G;
        *Aba_ -= G;
        *Abb_ += G;

        *ba_  -= I;
        *bb_  += I;

      }

    private:
      const std::string na_, nb_;
      const F f_;

      //system references
      circuit::entry_reference<float> Aaa_, Aab_, Aba_, Abb_;
      circuit::entry_reference<float> ba_, bb_;
      circuit::entry_reference<const float> xa_, xb_;
  };

  /*!
   * @brief class implementing linear resistance characteristics.
   *
   */
  class linear_resistance {
    public:

      static constexpr bool static_v   = true;
      static constexpr bool nonlinear_v = false;

      linear_resistance(float R) :
        G_( 1.0/R ) {}

      inline auto operator()(float v) const noexcept {
        return std::make_pair(G_*v, G_);
      }

    private:
      const float G_;
  };

  /*!
   * @brief class implementing shockley equation characteristics
   *
   */
  class diode_resistance {
    public:
      static constexpr bool static_v    = false;
      static constexpr bool nonlinear_v = true;

      diode_resistance(float IS, float N) :
        IS_{ IS },
        N_Vt_{ N * Vt },
        e_sat_ ( IS_*std::expm1(v_knee/N_Vt_) ),
        df_sat_( IS_*std::exp(v_knee/N_Vt_)/N_Vt_ ) { }

      inline auto operator()(float v) const noexcept -> std::pair<float,float> {

        if(v < v_knee) {
          const auto vnt = v/N_Vt_;
          const auto f  = IS_*std::expm1(vnt);
         const auto df = IS_*std::exp(vnt)/N_Vt_;
          return {f, df};
        } else {
          const auto f = e_sat_ + df_sat_*(v-v_knee);
          return {f, df_sat_};
        }

      }

    private:

      static constexpr float k = 1.3806504e-23;
      static constexpr float q = 1.602176487e-19; /* A s */
      static constexpr float Vt = k*300.0/q;

      static constexpr float v_knee = 0.8;

      const float IS_, N_Vt_, e_sat_, df_sat_;
  };

  using linear_resistor = resistor<linear_resistance>;
  using basic_diode     = resistor<diode_resistance>;

}		// -----  end of namespace rtspice::components  -----

#endif   // ----- #ifndef resistor_INC  -----
