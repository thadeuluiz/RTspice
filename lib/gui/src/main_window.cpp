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

#include "main_window.hpp"

#include <QLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include "circuit.hpp"
#include "netlist_parser.hpp"

using namespace std;
using namespace rtspice::gui;

main_window::main_window(QWidget* parent, Qt::WindowFlags flags) :
  QMainWindow{parent, flags} {

    layout()->setContentsMargins(200, 200, 200, 200);

    //create menu items
    file_menu_ = menuBar()->addMenu(tr("&File"));

    //create menu actions
    open_file_action_ = new QAction{tr("&Open..."), file_menu_};
    open_file_action_->setShortcut(QKeySequence::Open);
    connect(open_file_action_, &QAction::triggered, this, &main_window::open_file);

    close_file_action_ = new QAction{tr("&Close"), file_menu_};
    close_file_action_->setShortcut(QKeySequence::Close);
    close_file_action_->setEnabled(false);
    connect(close_file_action_, &QAction::triggered, this, &main_window::close_file);

    // register actions
    file_menu_->addAction(open_file_action_);
    file_menu_->addAction(close_file_action_);
  }

void main_window::open_file() {

  const auto file_name = QFileDialog::getOpenFileName(this,
      tr("Open Netlist"), "", tr("Netlist Files (*.net)"));

  QFile file{file_name};
  if(!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, tr(program_name),
        tr("Cannot open file %1:\n%2").arg(
          QDir::toNativeSeparators(file_name), file.errorString()));
    return;
  }

  parse_file(file_name, QTextStream{&file}.readAll());

}

void main_window::close_file() {
  delete circuit_;
  close_file_action_->setDisabled(true);
  open_file_action_->setEnabled(true);
}

void main_window::parse_file(const QString& file_name, const QString& file_content) {
  using namespace rtspice::parser;

  const auto f_content = file_content.toStdString();

  //load statement lines
  vector<string> statements;

  auto f_it = f_content.begin();
  bool good = qi::phrase_parse(f_it, f_content.end(),
      line_parser<string::const_iterator, qi::ascii::space_type>{},
      qi::ascii::space,
      statements);

  if(!good || f_it != f_content.end()) {

    QMessageBox::warning(this, tr(program_name),
        tr("Invalid syntax detected in netlist file%1:\n")
          .arg(QDir::toNativeSeparators(file_name)));

    return;
  }

  if(statements.empty()) {

    QMessageBox::warning(this, tr(program_name),
        tr("Netlist file %1 contains no statement.\n")
          .arg(QDir::toNativeSeparators(file_name)));

    return;
  }

  //map statements to components
  vector<component::ptr> components;
  auto statements_it = statements.cbegin();

  //first line is the circuit title
  const auto name = QString::fromStdString(*statements_it++);
  for(; statements_it != statements.cend(); ++statements_it) {
    const auto& statement = *statements_it;
    component::ptr component;

    auto statement_it = statement.begin();
    bool good = qi::phrase_parse(statement_it, statement.end(),
        statement_parser<string::const_iterator, qi::ascii::space_type>{},
        qi::ascii::space,
        component);
    if(!good || statement_it != statement.end()) {
      QMessageBox::warning(this, tr(program_name),
          tr("Invalid syntax detected in statement:\n\"%1\"\n in file %2:\n")
            .arg(QString::fromStdString(statement),
              QDir::toNativeSeparators(file_name)));
      return;
    }
    components.emplace_back(std::move(component));
  }

  //TODO: spawn circuit widget
  circuit_ = new circuit_widget{name, components, this};
  this->setCentralWidget(circuit_);

  open_file_action_->setDisabled(true);
  close_file_action_->setEnabled(true);

}
