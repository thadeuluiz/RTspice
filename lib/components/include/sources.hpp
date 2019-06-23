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

#include <unordered_map>
#include <atomic>

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

      virtual bool is_static()    const override { return F::static_v; }
      virtual bool is_dynamic()   const override { return F::dynamic_v; }
      virtual bool is_nonlinear() const override { return F::nonlinear_v; }

      template<class... Args>
      current_source(std::string id,
                     std::string na,
                     std::string nb,
                     Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        f_( std::forward<Args>(args)... ) {}

      virtual void register_(circuit::circuit &c) override {
        c.register_node(na_);
        c.register_node(nb_);
      }

      virtual void setup(circuit::circuit& c) override {
        ba_ = c.get_b(na_);
        bb_ = c.get_b(nb_);
        f_.setup(c);
      }

      virtual void fill() const noexcept override {
        const auto I = f_();
        *ba_ -= I;
        *bb_ += I;
      }

    private:
      const std::string na_, nb_;
      F f_;
      circuit::entry_reference<float> ba_, bb_;
  };

  /*!
   * @brief generalized independent voltage source
   *
   * General voltage source, where the value of the voltage is determined by
   * V(t). The class F must have an operator() accepting a time value. The
   * static or dynamic information is passed through member constants
   * called 'static_' and 'dynamic'.
   *
   */
  template<class F>
  class voltage_source : public component {
    public:

      virtual bool is_static()    const override { return F::static_v; }
      virtual bool is_dynamic()   const override { return F::dynamic_v; }
      virtual bool is_nonlinear() const override { return F::nonlinear_v; }

      template<class... Args>
      voltage_source(std::string id,
                     std::string na,
                     std::string nb,
                     Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nj_{ "J@" + id_ },
        f_( std::forward<Args>(args)... ) {}

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

        f_.setup(c);

      }

      virtual void fill() const noexcept override {

        const auto V = f_();

        *Aaj_ += 1.0;
        *Abj_ -= 1.0;
        *Aja_ -= 1.0;
        *Ajb_ += 1.0;
        *bj_  -= V;

      }

    private:
      const std::string na_, nb_, nj_;
      F f_;
      circuit::entry_reference<float> Aaj_, Abj_, Aja_, Ajb_, bj_;
  };

  /*!
   * @brief generalized voltage amplifier
   *
   * General voltage amplifier, where the value of the voltage is determined by
   * V(v). The class F must have an operator() accepting the dependent voltage value.
   * The linear or nonlinear information is passed through member constants
   * called 'static_v' and 'nonlinear_v'.
   *
   */
  template<class F>
  class vcvs : public component {
    public:
      virtual bool is_static()    const override { return F::static_v; }
      virtual bool is_dynamic()   const override { return F::dynamic_v; }
      virtual bool is_nonlinear() const override { return F::nonlinear_v; }

      template<class... Args>
      vcvs(std::string id,
           std::string na,
           std::string nb,
           std::string nc,
           std::string nd,
           Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        nj_{ "J@" + id_ },
        f_( std::forward<Args>(args)... ) {}

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

        xc_  = c.get_x(nc_);
        xd_  = c.get_x(nd_);

        bj_  = c.get_b(nj_);

        f_.setup(c);

      }

      virtual void fill() const noexcept override {

        const auto v = *xc_ - *xd_;
        const auto [f, df] = f_(v);

        const auto Av = df;
        const auto V  = f - Av*v;

        *Aaj_ += 1.0;
        *Abj_ -= 1.0;

        *Aja_ -= 1.0;
        *Ajb_ += 1.0;
        *Ajc_ += Av;
        *Ajd_ -= Av;

        *bj_  -= V;

      }

    private:
      const std::string na_, nb_, nc_, nd_, nj_;
      F f_;
      circuit::entry_reference<float> Aaj_, Abj_, Aja_, Ajb_, Ajc_, Ajd_;
      circuit::entry_reference<float> bj_;
      circuit::entry_reference<const float> xc_, xd_;
  };

  /*!
   * @brief generalized current amplifier
   *
   * General current amplifier, where the value of the current is determined by
   * I(i). The class F must have an operator() accepting the dependent current value.
   * The linear or nonlinear information is passed through member constants
   * called 'static_v' and 'nonlinear_v'.
   *
   */
  template<class F>
  class cccs : public component {
    public:
      virtual bool is_static()    const override { return F::static_v; }
      virtual bool is_dynamic()   const override { return F::dynamic_v; }
      virtual bool is_nonlinear() const override { return F::nonlinear_v; }

      template<class... Args>
      cccs(std::string id,
           std::string na,
           std::string nb,
           std::string nc,
           std::string nd,
           Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        nj_{ "J@" + id_ },
        f_( std::forward<Args>(args)... ) {}

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

        ba_  = c.get_b(na_);
        bb_  = c.get_b(nb_);

        xj_  = c.get_x(nj_);

        f_.setup(c);
      }

      virtual void fill() const noexcept override {

        const auto i = *xj_;
        const auto [f, df] = f_(i);

        const auto Ai = df;
        const auto I  = f - Ai*i;

        *Aaj_ += Ai;
        *Abj_ -= Ai;
        *Acj_ += 1.0;
        *Adj_ -= 1.0;

        *Ajc_ -= 1.0;
        *Ajd_ += 1.0;

        *ba_  -= I;
        *bb_  += I;

      }

    private:
      const std::string na_, nb_, nc_, nd_, nj_;
      F f_;
      circuit::entry_reference<float> Aaj_, Abj_, Acj_, Adj_, Ajc_, Ajd_;
      circuit::entry_reference<float> ba_, bb_;
      circuit::entry_reference<const float> xj_;
  };

  /*!
   * @brief generalized transconductor
   *
   * General transconductor, where the value of the voltage is determined by
   * I(v). The class F must have an operator() accepting the dependent voltage value.
   * The linear or nonlinear information is passed through member constants
   * called 'static_v' and 'nonlinear_v'.
   *
   */
  template<class F>
  class vccs : public component {
    public:
      virtual bool is_static()    const override { return F::static_v; }
      virtual bool is_dynamic()   const override { return F::dynamic_v; }
      virtual bool is_nonlinear() const override { return F::nonlinear_v; }

      template<class... Args>
      vccs(std::string id,
           std::string na,
           std::string nb,
           std::string nc,
           std::string nd,
           Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        f_( std::forward<Args>(args)... ) {}

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

        ba_  = c.get_b(na_);
        bb_  = c.get_b(nb_);

        xc_  = c.get_x(nc_);
        xd_  = c.get_x(nd_);

        f_.setup(c);

      }

      virtual void fill() const noexcept override {

        const auto v = *xc_ -*xd_;
        const auto [f, df] = f_(v);

        const auto Gm = df;
        const auto I = f - Gm*v;

        *Aac_ += Gm;
        *Aad_ -= Gm;
        *Abc_ -= Gm;
        *Abd_ += Gm;

        *ba_  -= I;
        *bb_  += I;

      }

    private:
      const std::string na_, nb_, nc_, nd_;
      F f_;
      circuit::entry_reference<float> Aac_, Aad_, Abc_, Abd_;
      circuit::entry_reference<float> ba_, bb_;
      circuit::entry_reference<const float> xc_, xd_;
  };

  /*!
   * @brief generalized transresistor
   *
   * General transresistor, where the value of the voltage is determined by
   * V(i). The class F must have an operator() accepting the dependent current value.
   * The linear or nonlinear information is passed through member constants
   * called 'static_v' and 'nonlinear_v'.
   *
   */
  template<class F>
  class ccvs : public component {
    public:
      virtual bool is_static()    const override { return F::static_v; }
      virtual bool is_dynamic()   const override { return F::dynamic_v; }
      virtual bool is_nonlinear() const override { return F::nonlinear_v; }

      template<class... Args>
      ccvs(std::string id,
           std::string na,
           std::string nb,
           std::string nc,
           std::string nd,
           Args&&... args) :
        component{ std::move(id) },
        na_{ std::move(na) },
        nb_{ std::move(nb) },
        nc_{ std::move(nc) },
        nd_{ std::move(nd) },
        nx_{ "Jx@" + id_ },
        ny_{ "Jy@" + id_ },
        f_( std::forward<Args>(args)... ) {}

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

        by_  = c.get_b(ny_);

        xx_  = c.get_x(nx_);

        f_.setup(c);

      }

      virtual void fill() const noexcept override {

        const auto j = *xx_;
        const auto [f, df] = f_(j);

        const auto Rm = df;
        const auto V  = f - Rm*j;

        *Aay_ += 1.0;
        *Aby_ -= 1.0;
        *Acx_ += 1.0;
        *Adx_ -= 1.0;
        *Axc_ -= 1.0;
        *Axd_ += 1.0;
        *Aya_ -= 1.0;
        *Ayb_ += 1.0;

        *Ayx_ += Rm;

        *by_  -= V;

      }

    private:
      const std::string na_, nb_, nc_, nd_, nx_, ny_;
      F f_;
      circuit::entry_reference<float> Aay_, Aby_, Acx_, Adx_,
                                      Axc_, Axd_, Aya_, Ayb_, Ayx_;

      circuit::entry_reference<float> by_;
      circuit::entry_reference<const float> xx_;
  };

  /*!
   * @brief simple transfer function implementing DC source characteristics
   *
   */
  class constant_function {
    public:
      static constexpr bool static_v   = true;
      static constexpr bool dynamic_v  = false;
      static constexpr bool nonlinear_v= false;

      constant_function(float val) :
        val_{ val } {}

      inline float operator()() const noexcept {
        return val_;
      }

      void setup(circuit::circuit&) {}

    private:
      const float val_;
  };

  /*!
   * @brief simple transfer function implementing sinusoidal source
   * characteristics
   */
  class sine_function {
    public:
      static constexpr bool static_v   = false;
      static constexpr bool dynamic_v  = true;
      static constexpr bool nonlinear_v= false;

      sine_function(float A, float f, float phase = 0.0) :
        A_{ A },
        w_( 8.0*std::atan(1.0)*f ), //2 pi f
        phi_( std::atan(1.0)*phase/45.0 ) {} //phase*pi/180

      inline float operator()() const noexcept {
        return A_*std::sin(*t_ * w_+ phi_);
      }

      void setup(circuit::circuit& c) {
        t_ = c.get_time();
      }

    private:
      const float A_, w_, phi_;
      const float *t_;
  };

  /*!
   * @brief source controlled externally, value is checked at every time frame
   */
  class external_function {
    public:
      static constexpr bool static_v   = false;
      static constexpr bool dynamic_v  = true;
      static constexpr bool nonlinear_v= false;

      external_function(std::string param) :
        param_name_{ std::move(param) } {}

      void setup(circuit::circuit& c) {
        val_ = &c.get_input(param_name_);
      }

      inline float operator()() const noexcept {
        return *val_;
      }

    private:
      const std::string param_name_;
      const float* val_;
  };

  /*!
   * @brief linear transfer characteristics
   */
  class linear_transfer {
    public:

      static constexpr bool static_v    = true;
      static constexpr bool dynamic_v   = false;
      static constexpr bool nonlinear_v = false;

      linear_transfer(float df) :
        df_( df ) {}

      void setup(circuit::circuit& c) {}

      inline auto operator()(float x) const noexcept {
        return std::make_pair(df_*x, df_);
      }

    private:
      const float df_;
  };

  using dc_current = current_source<constant_function>;
  using dc_voltage = voltage_source<constant_function>;

  using ac_current = current_source<sine_function>;
  using ac_voltage = voltage_source<sine_function>;

  using ext_current = current_source<external_function>;
  using ext_voltage = voltage_source<external_function>;

  using linear_vcvs = vcvs<linear_transfer>;
  using linear_vccs = vccs<linear_transfer>;
  using linear_cccs = cccs<linear_transfer>;
  using linear_ccvs = ccvs<linear_transfer>;

}		// -----  end of namespace rtspice::components  -----

#endif   // ----- #ifndef source_INC  -----
