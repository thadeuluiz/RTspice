/*!
 *    @file  circuit_widget.cpp
 *   @brief  circuit widget implementation
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

#include "circuit_widget.hpp"

#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "knob.hpp"

using namespace std;
using namespace rtspice::gui;

using rtspice::components::component;

circuit_widget::circuit_widget(const QString& name,
    const vector<component::ptr>& components, QWidget* parent) :
  QGroupBox{ name, parent },
  c_{ components } {

    setFlat(false);

    //prepare layout
    auto outer_box = new QHBoxLayout{this};

    //box for circuit definitions
    auto left_box  = new QVBoxLayout{};
    outer_box->addLayout(left_box);

    left_box->addWidget(
        new QLabel{QString{"Loaded circuit with %1 nodes."}
          .arg(c_.nodes().size())});

    left_box->addWidget(
        new QLabel{QString{"Modified admitance matrix has %1 non-zeros."}
          .arg(c_.entries().size())});

    auto knob_box = new QHBoxLayout{};
    left_box->addLayout(knob_box);

    for(auto&& [name, val] : c_.params()) {
      knob_box->addWidget( new knob{name, val} );
    }

    outer_box->addWidget( new jack_widget{c_, this} );

  }
