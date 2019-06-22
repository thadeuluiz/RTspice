/*!
 *    @file  knob.cpp
 *   @brief
 *
 *  <+DETAILED+>
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

#include "knob.hpp"

#include <cmath>

#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDial>

#include "circuit.hpp"

using namespace std;

namespace rtspice::gui {

knob_holder::knob_holder(circuit::circuit& c, QWidget* parent) :
  QGroupBox{"Controls", parent} {
    layout_ = new QHBoxLayout{this};

    for(auto&& [name, val]: c.params()) {
      auto knob_ = new knob{name, val, this};
      layout_->addWidget(knob_);
    }
  }


knob::knob(const string& name, atomic<float>& val, QWidget* parent) :
  QGroupBox{QString::fromStdString(name), parent},
  val_{val} {

    layout_ = new QVBoxLayout{this};

    dial_   = new QDial{this};
    dial_->setMinimumSize(150, 150);
    dial_->setMinimum(dial_min);
    dial_->setMaximum(dial_max);

    dial_->setWrapping(false);
    dial_->setNotchesVisible(true);

    dial_->setValue(dial_max/2);

    log_ = new QCheckBox{"Log", dial_};
    connect(log_, &QCheckBox::stateChanged, this, &knob::log_state);
    layout_->addWidget(log_);

    layout_->addWidget(dial_);

    connect(dial_, &QDial::valueChanged, this, &knob::set_value);

  }

void knob::set_value(int value) {

  const auto ratio = static_cast<float>(value)/dial_max;

  if(log_->isChecked())
    val_.store(a*pow(b, ratio) + c, memory_order_relaxed);
  else
    val_.store(ratio, memory_order_relaxed);
}

void knob::log_state(int) {
  set_value(dial_->value()); //recalculate the value
}

}		// -----  end of namespace rtspice::gui  -----
