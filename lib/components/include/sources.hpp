/*!
 *    @file  sources.hpp
 *   @brief independent and controlled sources
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
#include "circuit.hpp"

namespace rtspice::components {

  /*!
   * @brief generalized independent current source
   *
   * General current source, where the value of the current is determined by
   * I(t). The class F must have an operator() accepting a time value. The
   * static or dynamic information is passed through member constants
   * called 'static_' and 'dynamic'.
   */
  template<class F>
  class current_source : public component {
    public:

      virtual bool is_static()    const override { return F::static_; }
      virtual bool is_dynamic()   const override { return F::dynamic; }
      virtual bool is_nonlinear() const override { return false; }

      template<class... Args>
      current_source(std::string id,
                     std::string na,
                     std::string nb,
                     Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        f_{ std::forward<Args>(args)... } {}

      virtual void register_(circuit::circuit &c) override {
        c.register_node(na_);
        c.register_node(nb_);
      }

      virtual void setup(circuit::circuit& c) override {
        ba_ = c.get_b(na_);
        bb_ = c.get_b(nb_);
        t_ = c.get_time();
      }

      virtual void fill() override {
        const auto I = f_(*t_);
        *ba_ -= I;
        *bb_ += I;
      }

    private:
      const std::string na_, nb_;
      F f_;
      float *ba_, *bb_;
      const float *t_;
  };

  /*!
   * @brief generalized independent voltage source
   *
   * General voltage source, where the value of the voltage is determined by
   * I(t). The class F must have an operator() accepting a time value. The
   * static or dynamic information is passed through member constants
   * called 'static_' and 'dynamic'.
   *
   */
  template<class F>
  class voltage_source : public component {
    public:
      virtual bool is_static()    const override { return F::static_; }
      virtual bool is_dynamic()   const override { return F::dynamic; }
      virtual bool is_nonlinear() const override { return false; }

      template<class... Args>
      voltage_source(std::string id,
                     std::string na,
                     std::string nb,
                     Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nj_{ "@J" + id_ },
        f_(std::forward<Args>(args)...) {}

      virtual void register_(circuit::circuit &c) override {

        c.register_node(na_);
        c.register_node(nb_);
        c.register_node(nj_);

        c.register_entry({na_, nj_});
        c.register_entry({nb_, nj_});
        c.register_entry({nj_, na_});
        c.register_entry({nj_, nb_});

      }

      virtual void setup(circuit::circuit& c) override {

        Aaj_ = c.get_A({na_, nj_});
        Abj_ = c.get_A({nb_, nj_});
        Aja_ = c.get_A({nj_, na_});
        Ajb_ = c.get_A({nj_, nb_});
        bj_  = c.get_b(nj_);

        t_   = c.get_time();

      }

      virtual void fill() override {

        const auto V = f_(*t_);

        *Aaj_ += 1.0f;
        *Abj_ -= 1.0f;
        *Aja_ -= 1.0f;
        *Ajb_ += 1.0f;
        *bj_  -= V;

      }

    private:
      const std::string na_, nb_, nj_;
      F f_;
      float *Aaj_, *Abj_, *Aja_, *Ajb_, *bj_;
      const float* t_;
  };

  /*!
   * @brief basic linear voltage amplifier, optimized for one-time filling
   */
  class linear_vcvs : public component {
    public:
      virtual bool is_static()    const override { return true; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return false; }

      linear_vcvs(std::string id,
                  std::string na,
                  std::string nb,
                  std::string nc,
                  std::string nd,
                  float val) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        nj_{ "@J" + id_ },
        Av_{ val } {}

      virtual void register_(circuit::circuit &c) override {

        c.register_node(na_);
        c.register_node(nb_);
        c.register_node(nc_);
        c.register_node(nd_);
        c.register_node(nj_);

        c.register_entry({na_, nj_});
        c.register_entry({nb_, nj_});
        c.register_entry({nj_, na_});
        c.register_entry({nj_, nb_});
        c.register_entry({nj_, nc_});
        c.register_entry({nj_, nd_});

      }

      virtual void setup(circuit::circuit& c) override {

        Aaj_ = c.get_A({na_, nj_});
        Abj_ = c.get_A({nb_, nj_});
        Aja_ = c.get_A({nj_, na_});
        Ajb_ = c.get_A({nj_, nb_});
        Ajc_ = c.get_A({nj_, nc_});
        Ajd_ = c.get_A({nj_, nd_});

      }

      virtual void fill() override {

        *Aaj_ += 1.0f;
        *Abj_ -= 1.0f;

        *Aja_ -= 1.0f;
        *Ajb_ += 1.0f;
        *Ajc_ += Av_;
        *Ajd_ -= Av_;

      }

    private:
      const std::string na_, nb_, nc_, nd_, nj_;
      const float Av_;
      float *Aaj_, *Abj_, *Aja_, *Ajb_, *Ajc_, *Ajd_;
  };

  /*!
   * @brief basic linear current amplifier, optimized for one-time filling
   */
  class linear_cccs : public component {
    public:
      virtual bool is_static()    const override { return true; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return false; }

      linear_cccs(std::string id,
                  std::string na,
                  std::string nb,
                  std::string nc,
                  std::string nd,
                  float val) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        nj_{ "@J" + id_ },
        Ai_{ val } {}

      virtual void register_(circuit::circuit &c) override {

        c.register_node(na_);
        c.register_node(nb_);
        c.register_node(nc_);
        c.register_node(nd_);
        c.register_node(nj_);

        c.register_entry({na_, nj_});
        c.register_entry({nb_, nj_});
        c.register_entry({nc_, nj_});
        c.register_entry({nd_, nj_});
        c.register_entry({nj_, nc_});
        c.register_entry({nj_, nd_});

      }

      virtual void setup(circuit::circuit& c) override {

        Aaj_ = c.get_A({na_, nj_});
        Abj_ = c.get_A({nb_, nj_});
        Acj_ = c.get_A({nc_, nj_});
        Adj_ = c.get_A({nd_, nj_});
        Ajc_ = c.get_A({nj_, nc_});
        Ajd_ = c.get_A({nj_, nd_});

      }

      virtual void fill() override {

        *Aaj_ += Ai_;
        *Abj_ -= Ai_;
        *Acj_ += 1.0f;
        *Adj_ -= 1.0f;

        *Ajc_ -= 1.0f;
        *Ajd_ += 1.0f;

      }

    private:
      const std::string na_, nb_, nc_, nd_, nj_;
      const float Ai_;
      float *Aaj_, *Abj_, *Acj_, *Adj_, *Ajc_, *Ajd_;
  };

  /*!
   * @brief basic linear transconductor, optimized for one-time filling
   */
  class linear_vccs : public component {
    public:
      virtual bool is_static()    const override { return true; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return false; }

      linear_vccs(std::string id,
                  std::string na,
                  std::string nb,
                  std::string nc,
                  std::string nd,
                  float val) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        Gm_{ val } {}

      virtual void register_(circuit::circuit &c) override {

        c.register_node(na_);
        c.register_node(nb_);
        c.register_node(nc_);
        c.register_node(nd_);

        c.register_entry({na_, nc_});
        c.register_entry({na_, nd_});
        c.register_entry({nb_, nc_});
        c.register_entry({nb_, nd_});

      }

      virtual void setup(circuit::circuit& c) override {

        Aac_ = c.get_A({na_, nc_});
        Aad_ = c.get_A({na_, nd_});
        Abc_ = c.get_A({nb_, nc_});
        Abd_ = c.get_A({nb_, nd_});

      }

      virtual void fill() override {

        *Aac_ += Gm_;
        *Aad_ -= Gm_;
        *Abc_ -= Gm_;
        *Abd_ += Gm_;

      }

    private:
      const std::string na_, nb_, nc_, nd_;
      const float Gm_;
      float *Aac_, *Aad_, *Abc_, *Abd_;
  };

  /*!
   * @brief basic linear transresistor, optimized for one-time filling
   */
  class linear_ccvs : public component {
    public:
      virtual bool is_static()    const override { return true; }
      virtual bool is_dynamic()   const override { return false; }
      virtual bool is_nonlinear() const override { return false; }

      linear_ccvs(std::string id,
                  std::string na,
                  std::string nb,
                  std::string nc,
                  std::string nd,
                  float val) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        nx_{ "@Jx" + id_ },
        ny_{ "@Jy" + id_ },
        Rm_{ val } {}

      virtual void register_(circuit::circuit &c) override {

        c.register_node(na_);
        c.register_node(nb_);
        c.register_node(nc_);
        c.register_node(nd_);
        c.register_node(nx_);
        c.register_node(ny_);

        c.register_entry({na_, ny_});
        c.register_entry({nb_, ny_});

        c.register_entry({nc_, nx_});
        c.register_entry({nd_, nx_});

        c.register_entry({nx_, nc_});
        c.register_entry({nx_, nd_});

        c.register_entry({ny_, na_});
        c.register_entry({ny_, nb_});

        c.register_entry({ny_, nx_});

      }

      virtual void setup(circuit::circuit& c) override {

        Aay_ = c.get_A({na_, ny_});
        Aby_ = c.get_A({nb_, ny_});

        Acx_ = c.get_A({nc_, nx_});
        Adx_ = c.get_A({nd_, nx_});

        Axc_ = c.get_A({nx_, nc_});
        Axd_ = c.get_A({nx_, nd_});

        Aya_ = c.get_A({ny_, na_});
        Ayb_ = c.get_A({ny_, nb_});

        Ayx_ = c.get_A({ny_, nx_});

      }

      virtual void fill() override {

        *Aay_ += 1.0f;
        *Aby_ -= 1.0f;

        *Acx_ += 1.0f;
        *Adx_ -= 1.0f;

        *Axc_ -= 1.0f;
        *Axd_ += 1.0f;

        *Aya_ -= 1.0f;
        *Ayb_ += 1.0f;

        *Ayx_ += Rm_;

      }

    private:
      const std::string na_, nb_, nc_, nd_, nx_, ny_;
      const float Rm_;
      float *Aay_, *Aby_, *Acx_, *Adx_,
            *Axc_, *Axd_, *Aya_, *Ayb_, *Ayx_;
  };

  /*!
   * @brief simple transfer function implementing DC source characteristics
   *
   */
  class dc_function {
    public:
      static constexpr bool static_  = true;
      static constexpr bool dynamic  = false;

      dc_function(float val) :
        val_{ val } {}

      float operator()(float) {
        return val_;
      }

    private:
      const float val_;
  };

  /*!
   * @brief simple transfer function implementing sinusoidal source
   * characteristics
   */
  class sine_function {
    public:
      static constexpr bool static_  = false;
      static constexpr bool dynamic  = true;

      sine_function(float A, float f, float phase) :
        A_{ A },
        w_{ 8.0f*std::atan(1.0f)*f }, //2 pi f
        phi_{ std::atan(1.0f)*phase/45.0f } {} //phase*pi/180

    private:
      const float A_, w_, phi_;

  };

  using dc_current = current_source<dc_function>;
  using dc_voltage = voltage_source<dc_function>;

  using ac_current = current_source<sine_function>;
  using ac_voltage = voltage_source<sine_function>;



}		// -----  end of namespace rtspice::components  -----


#endif   // ----- #ifndef source_INC  -----
