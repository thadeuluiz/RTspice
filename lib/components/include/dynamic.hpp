/*!
 *    @file  dynamic.hpp
 *   @brief basic dynamic components
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

#ifndef  dynamic_INC
#define  dynamic_INC

#include "component.hpp"
#include "circuit.hpp"

namespace rtspice::components {

  /*!
   *  @brief  basic dynamic component template, optimized for
   *  modified nodal analysis
   */
  template<class F>
  class dynamic : public component {
    public:
      template<class... Args>
      dynamic(std::string id,
              std::string na,
              std::string nb,
              Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nj_{ "@J"+id_ },
        f_( std::forward<Args>(args)... ) {}

      virtual bool is_static()    const override { return false; }
      virtual bool is_dynamic()   const override { return F::dynamic_v; }
      virtual bool is_nonlinear() const override { return false; }

      virtual void register_(circuit::circuit& c) override {

        c.register_node(na_);
        c.register_node(nb_);
        c.register_node(nj_);

        c.register_entry({na_, nj_});
        c.register_entry({nb_, nj_});
        c.register_entry({nj_, na_});
        c.register_entry({nj_, nb_});
        c.register_entry({nj_, nj_});

      }

      virtual void setup(circuit::circuit& c) override {

        Aaj_ = c.get_A({na_, nj_});
        Abj_ = c.get_A({nb_, nj_});
        Aja_ = c.get_A({nj_, na_});
        Ajb_ = c.get_A({nj_, nb_});
        Ajj_ = c.get_A({nj_, nj_});

        bj_  = c.get_b(nj_);

        a_t0_ = c.get_state(na_);
        b_t0_ = c.get_state(nb_);
        j_t0_ = c.get_state(nj_);

        delta_t_ = c.get_delta_time();

      }

      virtual void fill() const noexcept override {

        const auto vt0 = *a_t0_ - *b_t0_;
        const auto jt0 = *j_t0_;

        const auto [R_dyn, V_dyn] = f_(vt0, jt0, *delta_t_);

        *Aaj_ += 1.0;
        *Abj_ -= 1.0;
        *Aja_ -= 1.0;
        *Ajb_ += 1.0;

        *Ajj_ += R_dyn;
        *bj_  -= V_dyn;

      }

    private:
      const std::string na_, nb_, nj_;
      const F f_;
      float *Aaj_, *Abj_, *Aja_, *Ajb_, *Ajj_;
      float *bj_;

      const float *a_t0_, *b_t0_, *j_t0_;
      const float *delta_t_;
  };

  /*!
   * @brief dynamic behaviour associated with a linear capacitor using a 
   * trapezoidal integration scheme.
   */
  class linear_capacitor_trapezoidal {
    public:

      static constexpr bool dynamic_v = true;

      linear_capacitor_trapezoidal(float C) :
        S_( 0.5 / C ) {}

      inline auto operator()(float v, float j, float delta_t) const noexcept {
        const auto R = delta_t*S_;
        const auto V = v + R*j;
        return std::make_pair(R, V);
      }

    private:
      const float S_;
  };

  /*!
   * @brief dynamic behaviour associated with a linear inductor using a 
   * trapezoidal integration scheme.
   */
  class linear_inductor_trapezoidal {
    public:
      static constexpr bool dynamic_v = true;

      linear_inductor_trapezoidal(float L) :
        L_( 2.0 * L ) {}

      inline auto operator()(float v, float j, float delta_t) const noexcept {
        const auto R = L_/delta_t;
        const auto V = v + R*j;
        return std::make_pair(R, -V); //source sign is reversed
      }
    private:
      const float L_;
  };

  using linear_capacitor = dynamic<linear_capacitor_trapezoidal>;
  using linear_inductor  = dynamic<linear_inductor_trapezoidal>;

}		// -----  end of namespace rtspice::components  -----

#endif   // ----- #ifndef dynamic_INC  -----

