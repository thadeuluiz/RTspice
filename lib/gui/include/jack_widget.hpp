/*!
 *    @file  jack_widget.hpp
 *   @brief  Qt widget providing interface with JACK
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


#ifndef  jack_widget_INC
#define  jack_widget_INC

#include "circuit.hpp"

#include <list>

#include <QGroupBox>
#include <QList>
#include <QString>

#include <jack/jack.h>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QProgressBar;
class QTimer;
class QFormLayout;
class QComboBox;

namespace rtspice::gui {

  class jack_widget : public QGroupBox {
    private:
      Q_OBJECT;

    public:
      jack_widget(circuit::circuit& c, QWidget* parent);
      ~jack_widget();

    private slots:
      void activate_(bool checked);
      void refresh_dsp_load_();

    private:
      class connection_widget_;

      //gui members
      QVBoxLayout        *layout_            = nullptr;
      QPushButton        *activate_button_   = nullptr;
      QTimer             *gui_refresh_timer_ = nullptr;
      QProgressBar       *dsp_load_bar_      = nullptr;
      connection_widget_ *connections_       = nullptr;

      //helper methods
      void init_client_();
      void register_ports_();
      void set_callbacks_();

      //circuit members
      circuit::circuit& circuit_;
      float delta_t_;
      QStringList node_names_;

      //jack members
      jack_client_t *client_          = nullptr;
      const char    **known_sources_  = nullptr;
      const char    **known_sinks_    = nullptr;

      struct input_port_ {
        const char                        *name   = nullptr;
        jack_port_t                       *handle = nullptr;
        float                             *entry  = nullptr;
        const jack_default_audio_sample_t *buffer = nullptr;
      };

      struct output_port_ {
        const char                            *name   = nullptr;
        jack_port_t                           *handle = nullptr;
        circuit::entry_reference<const float> entry;
        jack_default_audio_sample_t           *buffer = nullptr;
      };

      std::vector<input_port_> input_ports_;
      std::vector<output_port_> output_ports_;

      //jack callbacks
      static int process_callback(jack_nframes_t nframes, void* arg);
      static int sample_rate_callback(jack_nframes_t rate, void* arg);

      static constexpr auto program_name = "RTspice";
  };


  class jack_widget::connection_widget_ : public QGroupBox {
    private:
      Q_OBJECT;
    public:
      connection_widget_(jack_widget* parent);
      void connect_();

    private:
      friend class jack_widget;
      jack_widget *parent_ = nullptr;
      QHBoxLayout *layout_ = nullptr;
      QFormLayout *input_form_ = nullptr, *output_form_ = nullptr;
      QStringList known_sinks_, known_sources_;

      struct connection_ {
        jack_port_t *source;
        jack_port_t *sink;
      };
      std::list<connection_> connections_;
  };

}		// -----  end of namespace rtspice::gui  -----

#endif   // ----- #ifndef jack_widget_INC  -----
