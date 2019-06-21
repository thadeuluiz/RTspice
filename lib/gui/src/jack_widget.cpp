/*!
 *    @file  jack_widget.cpp
 *   @brief  implementation for basic jack widget
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

#include "jack_widget.hpp"

#include <QMessageBox>

using namespace std;
using namespace rtspice::gui;
using rtspice::circuit::circuit;

jack_widget::jack_widget(circuit::circuit& c, QWidget* parent) :
  QWidget{parent},
  circuit_{c} {

  //start jack
  jack_status_t status;
  client_ = jack_client_open("RTspice", JackNoStartServer, &status, nullptr);

  if(!client_) {
    QMessageBox::critical(this, tr(program_name),
        tr("Unable to open JACK client: status=%1\n").arg(status));
    return;
  }

  //get sample rate
  delta_t_ = 1.0/jack_get_sample_rate(client_);
  jack_set_sample_rate_callback(client_, &jack_widget::sample_rate_callback, this);

  register_ports_();

  jack_set_process_callback(client_, &jack_widget::process_callback, this);

  jack_activate(client_);

}

void jack_widget::register_ports_() {

  //collect all inputs
  for(auto&& [name, val] : circuit_.inputs()) {
    auto port = jack_port_register(client_, name.c_str(),
        JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    inputs_.emplace_back(port, &val);
  }
  input_buffers_.resize(inputs_.size());

  output_ = jack_port_register(client_, "output",
      JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

  output_entry_ = circuit_.get_x("OUT");
}


jack_widget::~jack_widget() {
  if(client_) jack_deactivate(client_);

  if(output_) jack_port_unregister(client_, output_);

  for(auto&& [port, _] : inputs_)
    jack_port_unregister(client_, port);

  if(client_) jack_client_close(client_);

}

int jack_widget::process_callback(jack_nframes_t n_frames, void* arg) {

  auto this_ = static_cast<jack_widget*>(arg);

  std::transform(this_->inputs_.begin(), this_->inputs_.end(),
      this_->input_buffers_.begin(),
      [n_frames](auto&& p){
        auto buffer = jack_port_get_buffer(p.first, n_frames);
        auto entry  = p.second;
        return make_pair(static_cast<jack_default_audio_sample_t*>(buffer), entry);
      });

  auto output_buffer = static_cast<jack_default_audio_sample_t*>(
      jack_port_get_buffer(this_->output_, n_frames));

  for(auto i = 0; i < n_frames; ++i) {

    for(auto&& [buf, entry] : this_->input_buffers_) *entry = buf[i];

    this_->circuit_.advance_(this_->delta_t_);

    output_buffer[i] = *this_->output_entry_;
  }

  return 0;
}

int jack_widget::sample_rate_callback(jack_nframes_t sample_rate, void* arg) {

  auto this_ = static_cast<jack_widget*>(arg);

  this_->delta_t_ = 1.0/sample_rate;

  return 0;
}
