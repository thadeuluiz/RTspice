/*!
 *    @file  main_window.hpp
 *   @brief
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


#ifndef  main_window_INC
#define  main_window_INC

#include <QMainWindow>

namespace rtspice::gui {

  class main_window : public QMainWindow {
    private:
      Q_OBJECT;
    public:
      main_window(QWidget* parent = nullptr, Qt::WindowFlags = {});

    protected:

    private slots:
      void open_file();

    private:
      QMenu* file_menu_;
			QAction* open_file_action_;

      static constexpr auto program_name = "RTspice";
  };

}		// -----  end of namespace rtspice::gui  ----- 

#endif   // ----- #ifndef main_window_INC  -----
