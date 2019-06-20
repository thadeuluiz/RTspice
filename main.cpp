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

#include <vector>
#include <atomic>
#include <unordered_map>

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include <QtWidgets>

#include <jack/jack.h>

#include "circuit.hpp"

#include "main_window.hpp"

using namespace std;
using namespace rtspice::components;
using rtspice::circuit::circuit;


unordered_map<string, atomic<float>> gExtParams{};


atomic<float> sample = 0.0;

struct process_data {
  jack_port_t *in, *out;
};

const char *client_name = "rtspice";
const char *server_name = nullptr;


int jack_process(jack_nframes_t nframes, void* data) {
  return 0;
}

constexpr auto process = [](jack_nframes_t nframes, void *arg) {
  auto& data = *static_cast<process_data*>(arg);

  jack_default_audio_sample_t *in, *out;

  in  = static_cast<jack_default_audio_sample_t*>(
      jack_port_get_buffer(data.in, nframes));
  out = static_cast<jack_default_audio_sample_t*>(
      jack_port_get_buffer(data.out, nframes));

  //auto res_ptr = cir.get_x("7");
  //for(auto i = 0; i < nframes; i++) {

  //  sample.store(in[i]);
  //  cir.advance_(1.0 / sample_rate );
  //  out[i] = *res_ptr;

  //}

  memcpy(out, in,
      nframes*sizeof(jack_default_audio_sample_t));

  return 0;

};





  //jack_options_t options = JackNoStartServer;
  //jack_status_t status;

  //auto client = jack_client_open(client_name, options, &status, server_name);
  //if(!client) {
  //  fprintf(stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
  //  if(status & JackServerFailed) {
  //    fprintf(stderr, "Unable to connect to JACK server\n");
  //  }
  //  exit(1);
  //}
  //if(status & JackServerStarted) {
  //  fprintf(stderr, "JACK server started\n");
  //}
  //if(status & JackNameNotUnique) {
  //  client_name = jack_get_client_name(client);
  //  fprintf(stderr, "unique name \"%s\" assigned\n", client_name);
  //}

  //const auto sample_rate = jack_get_sample_rate(client);
  //printf("sample_rate = %d\n", sample_rate);

  //process_data p_data;

  //jack_set_process_callback(client, jack_process, &p_data);
  //jack_on_shutdown(client, [](void* arg){ exit(1); }, nullptr);


  ///* create two ports */
  //p_data.in = jack_port_register(client, "in", JACK_DEFAULT_AUDIO_TYPE,
  //    JackPortIsInput, 0);
  //if(!p_data.in) {
  //  fprintf(stderr, "Unable to aquire input port\n");
  //  exit(1);
  //}
  //p_data.out = jack_port_register(client, "out", JACK_DEFAULT_AUDIO_TYPE,
  //    JackPortIsOutput, 0);
  //if(!p_data.out) {
  //  fprintf(stderr, "Unable to aquire output port\n");
  //  exit(1);
  //}

  //if(jack_activate(client)) {
  //  fprintf(stderr, "cannot activate client");
  //  exit(1);
  //}

  ////find and connect available system ports
  //const char **sys_ports = nullptr;

  //sys_ports = jack_get_ports(client, nullptr, nullptr,
  //    JackPortIsPhysical|JackPortIsOutput);
  //if(sys_ports == nullptr) {
  //  fprintf(stderr, "no physical capture ports\n");
  //  exit(1);
  //}

  //if(jack_connect(client, sys_ports[0], jack_port_name(p_data.in))) {
  //  fprintf(stderr, "cannot connect input ports\n");
  //}

  //sys_ports = jack_get_ports(client, nullptr, nullptr,
  //    JackPortIsPhysical|JackPortIsInput);
  //if(sys_ports == nullptr) {
  //  fprintf(stderr, "no physical playback ports\n");
  //  exit(1);
  //}
  //if(jack_connect(client, jack_port_name(p_data.out), sys_ports[0])) {
  //  fprintf(stderr, "cannot connect output ports\n");
  //}
  //free(sys_ports);


  //sleep(-1);

  //jack_client_close(client);
  //exit(0);


  class dial_group : public QGroupBox {
    private:
      Q_OBJECT;

    public:
      dial_group(const QString& title, QWidget* parent = nullptr) :
        QGroupBox{title, parent} {

          auto layout = new QHBoxLayout;

          for(auto i = 0; i < 3; i++) {

            auto box = new QVBoxLayout{};
            box->setAlignment(Qt::AlignCenter);

            auto label = new QLabel{QString{"dial %1"}.arg(i)};
            label->setAlignment(Qt::AlignCenter);
            box->addWidget(label);

            auto dial = new QDial{};
            dial->setFocusPolicy(Qt::StrongFocus);
            box->addWidget(dial);
            layout->addLayout(box);
          }

          setLayout(layout);

        }
  };

#include "main.moc"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    auto main_window = rtspice::gui::main_window{};

    main_window.show();


    return app.exec();
}

