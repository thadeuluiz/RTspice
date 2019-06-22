/*!
 *    @file  circuit_widget.hpp
 *   @brief  Qt-based circuit holder
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/20/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */


#ifndef  circuit_widget_INC
#define  circuit_widget_INC

#include "circuit.hpp"

#include <vector>

#include <QWidget>

class QGridLayout;
class QGroupBox;

namespace rtspice::gui {

  class jack_widget;
  class knob_holder;

  class circuit_widget : public QWidget {
    private:
      Q_OBJECT;

    public:
      circuit_widget(const QString& name,
          const std::vector<components::component::ptr>& components,
          QWidget* parent);

      ~circuit_widget();

    private:
      circuit::circuit c_;
      QGridLayout   *layout_    = nullptr;
      QGroupBox     *info_box_  = nullptr;
      knob_holder   *knobs_     = nullptr;
      jack_widget   *jack_info_ = nullptr;

  };

}		// -----  end of namespace rtspice::gui  -----

#endif   // ----- #ifndef circuit_widget_INC  -----
