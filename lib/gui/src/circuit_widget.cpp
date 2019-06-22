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

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

#include "jack_widget.hpp"
#include "knob.hpp"

using namespace std;
using namespace rtspice::gui;

using rtspice::components::component;

circuit_widget::circuit_widget(const QString& name,
    const vector<component::ptr>& components, QWidget* parent) :
  QWidget{ parent },
  c_{ components } {

    //prepare layout
    layout_ = new QGridLayout{this};
    setLayout(layout_);

    info_box_ = new QGroupBox{"Circuit information", this};
    info_box_->setLayout(new QVBoxLayout{info_box_});
    layout_->addWidget(info_box_, 0, 0, 1, 1);

    info_box_->layout()->addWidget(
        new QLabel{QString{"%1."}.arg(name), info_box_});

    info_box_->layout()->addWidget(
      new QLabel{QString{"Loaded circuit with %1 nodes."}
        .arg(c_.nodes().size()), info_box_});

    info_box_->layout()->addWidget(
      new QLabel{QString{"Modified admitance matrix has %1 non-zeros."}
        .arg(c_.entries().size()), info_box_});


    knobs_ = new knob_holder{c_, this};
    layout_->addWidget(knobs_, 1, 0, 1, 2);

    jack_info_ = new jack_widget{c_, this};

    layout_->addWidget(jack_info_, 0, 1, 1, 1);
  }

circuit_widget::~circuit_widget() {
  delete jack_info_; //jack must be destroyed first
}
