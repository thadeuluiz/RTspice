/*!
 *    @file  main_window.cpp
 *   @brief  main application window implementation
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

#include <iostream>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include "main_window.hpp"

using namespace rtspice::gui;

main_window::main_window(QWidget* parent, Qt::WindowFlags flags) :
  QMainWindow{parent, flags} {

    //create menu items
    file_menu_ = menuBar()->addMenu(tr("File"));

    //create menu actions
    open_file_action_ = new QAction{tr("Open..."), file_menu_};
    open_file_action_->setShortcut(QKeySequence::Open);
    connect(open_file_action_, &QAction::triggered, this, &main_window::open_file);


    // register actions
    file_menu_->addAction(open_file_action_);


  }

void main_window::open_file() {

  const auto file_name = QFileDialog::getOpenFileName(this,
    tr("Open Netlist"), "", tr("Netlist Files (*.net)"));

  QFile file{file_name};
  if(!file.open(QFile::ReadOnly | QFile::Text)) {

    QMessageBox::warning(this, tr(program_name),
        tr("Cannot open file %1:\n%2").arg(
          QDir::toNativeSeparators(file_name),
          file.errorString()));

    return;
  }

  const auto content = QTextStream{&file}.readAll();

}
