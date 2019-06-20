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

#include <QString>
#include <QVBoxLayout>
#include <QLabel>
#include <QDial>

using namespace std;
using namespace rtspice::gui;


knob::knob(const string& name, atomic<float>& val, QWidget* parent) :
  QWidget{parent},
  val_{val} {

    layout_ = new QVBoxLayout{this};
    label_  = new QLabel{QString::fromStdString(name)};
    dial_   = new QDial{};

    label_->setAlignment(Qt::AlignCenter);

    dial_->setMinimum(dial_min);
    dial_->setMaximum(dial_max);

    dial_->setWrapping(false);
    dial_->setNotchesVisible(true);

    layout_->addWidget(label_);
    layout_->addWidget(dial_);

    connect(dial_, &QDial::valueChanged, this, &knob::set_value);

  }

void knob::set_value(int value) {
  val_.store(static_cast<float>(value)/dial_max);
}
