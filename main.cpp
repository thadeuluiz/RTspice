/*!
 *    @file  main.cpp
 *   @brief
 *
 *  <+DETAILED+>
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/14/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include <vector>
#include <atomic>

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include <jack/jack.h>

#include "circuit.hpp"
#include "sources.hpp"
#include "resistor.hpp"
#include "dynamic.hpp"
#include "opamp.hpp"


using namespace std;

using namespace rtspice::components;

using rtspice::circuit::circuit;


constexpr float dist = 250.0e+3;
constexpr float tone = 10.0e+3;

atomic<float> sample = 0.0;

jack_port_t *input_port  = nullptr;
jack_port_t *output_port = nullptr;
jack_client_t *client    = nullptr;

jack_nframes_t sample_rate = 0;

int main(int argc, char *argv[]) {

  const char **ports;
  const char *client_name = "rtspice";
  const char *server_name = nullptr;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  client = jack_client_open(client_name, options, &status, server_name);

  if(client == nullptr) {

    fprintf(stderr, "jack_client_open() failed, "
        "status = 0x%2.0x\n", status);

    if(status & JackServerFailed) {

      fprintf(stderr, "Unable to connect to JACK server\n");

    }
    exit(1);
  }
  if(status & JackServerStarted) {

    fprintf(stderr, "JACK server started\n");

  }
  if(status & JackNameNotUnique) {

    client_name = jack_get_client_name(client);
    fprintf(stderr, "unique name `%s' assigned\n", client_name);

  }

  sample_rate = jack_get_sample_rate(client);
  printf("sample_rate = %d\n", sample_rate);

  vector<component::ptr> components {

    //clipping section
    //input
    make_component<ext_voltage>      ("V1", "0",    "3",      sample), //100 mV, 440 Hz

    //opamp
    make_component<ideal_opamp>     ("U1A","1",     "0",     "2", "3"),

    //feedback
    make_component<linear_resistor> ("R4", "A",     "0",     4.7e3),
    make_component<linear_capacitor>("C3", "2",     "A",     47.0e-9),

    make_component<linear_capacitor>("C4", "1",     "2",     51.0e-12),
    make_component<basic_diode>     ("D1", "1",     "2",     4.352e-9f, 1.906f),  //1n4148
    make_component<basic_diode>     ("D2", "2",     "1",     4.352e-9f, 1.906f),  //1n4148
    make_component<linear_resistor> ("R6", "2",     "1",     51.0e3 + dist),

    //tone section
    make_component<linear_resistor> ("R7", "1",     "5",     4.7e3),
    make_component<linear_capacitor>("C5", "5",     "0",     0.22e-6),
    make_component<linear_resistor> ("R9", "5",     "0",     10.0e3),

    make_component<ideal_opamp>     ("U1B","7",     "0",     "5", "6"),

    make_component<linear_resistor> ("R8", "0",     "B",     220.0),
    make_component<linear_capacitor>("C6", "B",     "T",     0.22e-6),
    make_component<linear_resistor> ("RTa","5",     "T",     20.0e3 - tone),
    make_component<linear_resistor> ("RTb","T",     "6",     tone),

    make_component<linear_resistor> ("R11","6",     "7",     1.0e3),

  };
  circuit c{components};


  constexpr auto process = [](jack_nframes_t nframes, void *arg) {
    auto& cir = *static_cast<circuit*>(arg);

    jack_default_audio_sample_t *in, *out;

    in  = static_cast<jack_default_audio_sample_t*>(
        jack_port_get_buffer(input_port, nframes));
    out = static_cast<jack_default_audio_sample_t*>(
        jack_port_get_buffer(output_port, nframes));

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
  jack_set_process_callback(client, process, &c);

  constexpr auto shutdown = [](void *arg) { exit(1); };
  jack_on_shutdown(client, shutdown, 0);


  /* create two ports */
  input_port = jack_port_register(client, "input",
      JACK_DEFAULT_AUDIO_TYPE,
      JackPortIsInput, 0);
  output_port = jack_port_register(client, "output",
      JACK_DEFAULT_AUDIO_TYPE,
      JackPortIsOutput, 0);

  if((input_port == nullptr) || (output_port == nullptr)) {
    fprintf(stderr, "no more JACK ports available\n");
    exit(1);
  }

  /* Tell the JACK server that we are ready to roll.  Our
   * process() callback will start running now. */

  if(jack_activate(client)) {
    fprintf(stderr, "cannot activate client");
    exit(1);
  }

  /* Connect the ports.  You can't do this before the client is
   * activated, because we can't make connections to clients
   * that aren't running.  Note the confusing (but necessary)
   * orientation of the driver backend ports: playback ports are
   * "input" to the backend, and capture ports are "output" from
   * it.
   */

  ports = jack_get_ports(client, nullptr, nullptr,
      JackPortIsPhysical|JackPortIsOutput);
  if(ports == nullptr) {
    fprintf(stderr, "no physical capture ports\n");
    exit(1);
  }

  if(jack_connect(client, ports[0], jack_port_name(input_port))) {
    fprintf(stderr, "cannot connect input ports\n");
  }

  free(ports);

  ports = jack_get_ports(client, nullptr, nullptr,
      JackPortIsPhysical|JackPortIsInput);
  if(ports == nullptr) {
    fprintf(stderr, "no physical playback ports\n");
    exit(1);
  }

  if(jack_connect(client, jack_port_name(output_port), ports[0])) {
    fprintf(stderr, "cannot connect output ports\n");
  }

  free(ports);

  /* keep running until stopped by the user */

  sleep(-1);

  /* this is never reached but if the program
     had some other way to exit besides being killed,
     they would be important to call.
     */

  jack_client_close(client);
  exit(0);
}

