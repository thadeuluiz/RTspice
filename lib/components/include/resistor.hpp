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

#include "circuit.hpp"
#include "component.hpp"

namespace rtspice::components {

  using real_t = circuit::circuit::real_t;

  /*!
   * @brief basic resistor class
   *
   * optimized when compared to nonlinear version, as filling only happens once,
   * and there is no need to evaluate f or df
  class linear_resistor : public component {
    public:

      virtual bool is_static()    const override { return true; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return false; }

      linear_resistor(std::string id,
                      std::string np,
                      std::string nm,
                      float val) :
        component{std::move(id)},
        na_{std::move(np)},
        nb_{std::move(nm)},
        G_{1.0f/val} {}

      virtual void register_(circuit::circuit& circuit) override {

        circuit.register_node(na_);
        circuit.register_node(nb_);

        circuit.register_entry({na_, na_});
        circuit.register_entry({na_, nb_});
        circuit.register_entry({nb_, na_});
        circuit.register_entry({nb_, nb_});

      }

      virtual void setup(circuit::circuit& circuit) override {

        Aaa_ = circuit.get_A({na_, na_});
        Aab_ = circuit.get_A({na_, nb_});
        Aba_ = circuit.get_A({nb_, na_});
        Abb_ = circuit.get_A({nb_, nb_});

      }

      virtual void fill() override {
        *Aaa_ += G_;
        *Aab_ -= G_;
        *Aba_ -= G_;
        *Abb_ += G_;
      }

    private:
      const std::string na_, nb_;
      const float G_;
      float *Aaa_, *Aab_, *Aba_, *Abb_;
  };
   */

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

      virtual bool is_static()    const override { return F::static_; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return F::nonlinear; }

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

      virtual void fill() override {

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
      F f_;

      //system references
      real_t *Aaa_, *Aab_, *Aba_, *Abb_;
      real_t *ba_, *bb_;
      const real_t *xa_, *xb_;
  };

  /*!
   * @brief class implementing linear resistance characteristics.
   *
   */
  class linear_resistance {
    public:

      static constexpr bool static_   = true;
      static constexpr bool nonlinear = false;

      linear_resistance(real_t R) :
        G_( 1.0/R ) {}

      auto operator()(real_t v) const {
        return std::make_pair(G_*v, G_);
      }

    private:
      const real_t G_;
  };

  class diode_resistance {
    public:
      static constexpr bool static_   = false;
      static constexpr bool nonlinear = true;

      diode_resistance(real_t IS, real_t N) :
        IS_{ IS },
        N_Vt_{ N * Vt },
        e_sat_ ( IS_*(std::exp(v_knee/N_Vt_) - 1.0) ),
        df_sat_( IS_*std::exp(v_knee/N_Vt_)/N_Vt_ ) { }

      auto operator()(real_t v) const -> std::pair<real_t,real_t> {

        if(v < v_knee) {

          const auto e  = std::exp(v/N_Vt_);
          const auto f  = IS_*(e-1.0);
          const auto df = IS_*e/N_Vt_;

          return {f, df};
        } else {
          const auto f = e_sat_ + df_sat_*(v-v_knee);
          return {f, df_sat_};
        }

      }

    private:

      static constexpr real_t k = 1.3806504e-23;
      static constexpr real_t q = 1.602176487e-19; /* A s */
      static constexpr real_t Vt = k*300.0/q;

      static constexpr real_t v_knee = 0.75;

      const real_t IS_, N_Vt_, e_sat_, df_sat_;
  };

  using linear_resistor = resistor<linear_resistance>;
  using basic_diode     = resistor<diode_resistance>;

}		// -----  end of namespace rtspice::components  -----

#endif   // ----- #ifndef resistor_INC  -----
