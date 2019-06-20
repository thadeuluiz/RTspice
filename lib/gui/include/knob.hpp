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

#include <QWidget>

class QLabel;
class QDial;

namespace rtspice::gui {
  
  class knob : public QWidget {
    private:
      Q_OBJECT;
    public:
      knob(const std::string& name, std::atomic<float>& val, QWidget* parent = nullptr);

    private slots:
      void set_value(int val);

    private:

      static constexpr auto dial_min = 0;
      static constexpr auto dial_max = 100;

      QLayout* layout_;
      QLabel*  label_;
      QDial*   dial_;
      std::atomic<float>& val_;
  };

}		// -----  end of namespace rtspice::gui  -----



#endif   // ----- #ifndef knob_INC  -----
