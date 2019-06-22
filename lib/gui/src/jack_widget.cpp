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
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <QFormLayout>
#include <QComboBox>

using namespace std;
using namespace rtspice::gui;
using rtspice::circuit::circuit;

jack_widget::jack_widget(circuit::circuit& c, QWidget* parent) :
  QGroupBox{"Jack Settings", parent},
  circuit_{c} {

    init_client_();
    set_callbacks_();
    register_ports_();

    layout_ = new QVBoxLayout{this};

    activate_button_ = new QPushButton{"Activate", this};
    activate_button_->setCheckable(true);
    connect(activate_button_, &QPushButton::toggled, this, &jack_widget::activate_);
    layout_->addWidget(activate_button_);

    gui_refresh_timer_ = new QTimer{this};
    gui_refresh_timer_->setInterval(1000);

    dsp_load_bar_ = new QProgressBar{this};
    dsp_load_bar_->setFormat("DSP Load: %p%");
    layout_->addWidget(dsp_load_bar_);
    connect(gui_refresh_timer_, &QTimer::timeout, this, &jack_widget::refresh_dsp_load_);
    gui_refresh_timer_->start();

    connections_ = new connection_widget_{this};
    layout_->addWidget(connections_);

}

void jack_widget::init_client_() {

  jack_status_t status;
  client_ = jack_client_open(program_name, JackNoStartServer, &status, nullptr);

  if(!client_) {
    QMessageBox::critical(this, tr(program_name),
        tr("Unable to open JACK client: status = %1.\n").arg(status));
    return;
  }

  known_sources_ = jack_get_ports(client_, nullptr, nullptr, JackPortIsOutput);
  known_sinks_   = jack_get_ports(client_, nullptr, nullptr, JackPortIsInput);

}

void jack_widget::set_callbacks_() {

  jack_set_process_callback(client_, &jack_widget::process_callback, this);
  jack_set_sample_rate_callback(client_, &jack_widget::sample_rate_callback, this);

}

void jack_widget::register_ports_() {

  for(auto&& [name, val] : circuit_.inputs()) {
    auto port = input_port_{};

    port.name   = name.c_str();
    port.handle = jack_port_register(client_, name.c_str(),
        JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    port.entry  = &val;
    port.buffer = nullptr;

    input_ports_.emplace_back(std::move(port));
  }

  for(auto&& [name, val] : circuit_.outputs()) {
    auto port = output_port_{};

    port.name   = name.c_str();
    port.handle = jack_port_register(client_, name.c_str(),
        JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    port.entry  = val;
    port.buffer = nullptr;

    output_ports_.emplace_back(std::move(port));
  }

}

jack_widget::~jack_widget() {

  jack_deactivate(client_); //also disconnect ports

  free(known_sources_);
  free(known_sinks_);

  for(auto&& p : input_ports_)
    jack_port_unregister(client_, p.handle);

  for(auto&& p : output_ports_)
    jack_port_unregister(client_, p.handle);

  jack_client_close(client_);

}

void jack_widget::activate_(bool checked) {
  if(checked) {
    jack_activate(client_);
    connections_->connect_();
    connections_->setDisabled(true);
  }
  else {
    jack_deactivate(client_);
    connections_->setDisabled(false);
  }
}

void jack_widget::refresh_dsp_load_() {
  const auto load = jack_cpu_load(client_);
  dsp_load_bar_->setValue(load);
}

int jack_widget::process_callback(jack_nframes_t n_frames, void* arg) {

  auto this_  = static_cast<jack_widget*>(arg);
  auto& iports = this_->input_ports_;
  auto& oports = this_->output_ports_;

  const auto get_buffer = [n_frames](auto& p) {
    p.buffer = static_cast<jack_default_audio_sample_t*>(
        jack_port_get_buffer(p.handle, n_frames));
  };

  //load buffer pointers
  for_each(begin(iports), end(iports), get_buffer);
  for_each(begin(oports), end(oports), get_buffer);

  for(auto i = 0; i < n_frames; ++i) {

    for(auto& p : iports) *p.entry = p.buffer[i];
    this_->circuit_.advance_(this_->delta_t_);
    for(auto& p : oports) p.buffer[i] = *p.entry;

  }
  return 0;

}

int jack_widget::sample_rate_callback(jack_nframes_t sample_rate, void* arg) {
  auto this_ = static_cast<jack_widget*>(arg);
  this_->delta_t_ = 1.0/sample_rate;
  return 0;
}

jack_widget::connection_widget_::connection_widget_(jack_widget* parent) :
  QGroupBox{ "Connections", parent },
  parent_{ parent }{
    layout_ = new QHBoxLayout{this};
    setLayout(layout_);

    //prepare input connection forms
    known_sources_.append("Disconnected");
    for(auto it = parent->known_sources_; *it != nullptr; ++it)
      known_sources_.append(*it);

    input_form_ = new QFormLayout{};
    for(auto& p : parent->input_ports_) {
      auto& conn = connections_.emplace_back();
      conn.sink = p.handle;

      auto combo = new QComboBox{};
      combo->addItems(known_sources_);

      connect(combo, QOverload<const QString&>::of(&QComboBox::activated),
          [&conn, parent](const QString& name) {
        const auto name_ = name.toStdString();
        conn.source = jack_port_by_name(parent->client_, name_.c_str());
      });

      input_form_->addRow(p.name, combo);
    }
    layout_->addLayout(input_form_);

    //and output connection forms too...
    known_sinks_.append("Disconnected");
    for(auto it = parent->known_sinks_; *it != nullptr; ++it)
      known_sinks_.append(*it);
    output_form_ = new QFormLayout{};
    for(auto& p : parent->output_ports_) {
      auto& conn = connections_.emplace_back();
      conn.source = p.handle;

      auto combo = new QComboBox{};
      combo->addItems(known_sinks_);

      connect(combo, QOverload<const QString&>::of(&QComboBox::activated),
          [&conn, parent](const QString& name) {
        const auto name_ = name.toStdString();
        conn.sink = jack_port_by_name(parent->client_, name_.c_str());
      });

      output_form_->addRow(p.name, combo);
    }
    layout_->addLayout(output_form_);

  }

void jack_widget::connection_widget_::connect_() {

  for(auto& c: connections_) {
    if(c.source != nullptr && c.sink != nullptr)
      jack_connect(parent_->client_, jack_port_name(c.source), jack_port_name(c.sink));
  }

}
