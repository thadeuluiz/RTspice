/*!
 *    @file  main.cpp
 *   @brief
 *
 *  <+DETAILED+>
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/14/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include "main_window.hpp"

#include <QApplication>

using namespace std;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    auto main_window = rtspice::gui::main_window{};

    main_window.show();

    return app.exec();
}
