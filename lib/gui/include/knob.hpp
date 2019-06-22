/*!
 *    @file  knob.hpp
 *   @brief  gui knob loading values to an atomic float value
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/19/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */


#ifndef  knob_INC
#define  knob_INC

#include <string>
#include <atomic>

#include <QGroupBox>

namespace rtspice::circuit {

  class circuit;

}

class QLabel;
class QDial;
class QCheckBox;

namespace rtspice::gui {

  class knob : public QGroupBox {
    private:
      Q_OBJECT;
    public:
      knob(const std::string& name, std::atomic<float>& val, QWidget* parent = nullptr);

    private slots:
      void set_value(int val);
      void log_state(int state);

    private:

      QLayout   *layout_;
      QCheckBox *log_;
      QDial     *dial_;
      std::atomic<float>& val_;


      //knob settings
      static constexpr auto dial_min = 1;
      static constexpr auto dial_max = 100;

      static constexpr auto ym = 0.1; //10% when log dial is at 50%
      static constexpr auto b = (1.0/ym - 1.0)*(1.0/ym - 1.0);
      static constexpr auto a = 1.0/(b - 1.0);
      static constexpr auto c = -a;
  };


  class knob_holder : public QGroupBox {
    private:
      Q_OBJECT;
    public:
      knob_holder(circuit::circuit& c, QWidget* parent);

    private:
      QLayout *layout_;
  };

}		// -----  end of namespace rtspice::gui  -----



#endif   // ----- #ifndef knob_INC  -----
