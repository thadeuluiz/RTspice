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

#include <QWidget>

#include <jack/jack.h>

namespace rtspice::gui {

  class jack_widget : public QWidget {
    private:
      Q_OBJECT;

    public:
      jack_widget(circuit::circuit& c, QWidget* parent);
      ~jack_widget();

    private:
      //helper methods
      void register_ports_();

      //basic members
      circuit::circuit& circuit_;
      float delta_t_;

      //jack members
      jack_client_t *client_;
      std::vector<std::pair<jack_port_t*, float*>> inputs_;
      std::vector<std::pair<jack_default_audio_sample_t*, float*>> input_buffers_;

      jack_port_t *output_;
      circuit::entry_reference<const float> output_entry_;

      //jack callbacks
      static int process_callback(jack_nframes_t nframes, void* arg);
      static int sample_rate_callback(jack_nframes_t rate, void* arg);

      static constexpr auto program_name = "RTspice";
  };

}		// -----  end of namespace rtspice::gui  -----

#endif   // ----- #ifndef jack_widget_INC  -----
