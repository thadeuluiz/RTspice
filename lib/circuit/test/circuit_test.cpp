/*!
 *    @file  circuit_test.cpp
 *   @brief basic circuit functionality test
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/11/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include <string>
#include <chrono>

#include <fstream>
#include <iterator>

#include <catch2/catch.hpp>


#include "circuit.hpp"
#include "resistor.hpp"
#include "sources.hpp"
#include "dynamic.hpp"
#include "opamp.hpp"


using namespace std::string_literals;
using namespace rtspice::components;

using std::vector;

using std::chrono::high_resolution_clock;
using std::chrono::nanoseconds;

using rtspice::circuit::circuit;

SCENARIO("circuit initialization", "[circuit]") {

  GIVEN("a component initialization vector") {

    vector<component::ptr> components {
      make_component<linear_resistor>("R1", "1", "2", 1.0f),
      make_component<dc_voltage>     ("V1", "1", "0", 1.0f),
      make_component<linear_resistor>("R2", "2", "0", 1.0f),
      make_component<linear_resistor>("R3", "0", "3", 1e3f),
      make_component<linear_resistor>("R4", "3", "4", 1e3f),
      make_component<linear_resistor>("R5", "4", "5", 1e3f),
      make_component<linear_resistor>("R6", "5", "6", 1e3f),
      make_component<linear_resistor>("R7", "6", "7", 1e3f),
      make_component<linear_resistor>("R8", "7", "8", 1e3f),
      make_component<linear_resistor>("R9", "8", "9", 1e3f),
      make_component<linear_resistor>("RA", "9", "0", 1e3f),
    };

    THEN("initialization succeeds"){
      circuit c{components};

      auto&& nodes = c.nodes();

      REQUIRE(nodes.find("0") == nodes.end());
      REQUIRE(nodes.find("1") != nodes.end());
      REQUIRE(nodes.find("2") != nodes.end());
      //REQUIRE(nodes.size()    == 3); //for V1 adds an extra node
      REQUIRE(nodes.at("1")   == 0);
      REQUIRE(nodes.at("2")   == 1);

    }

    THEN("stepping succeeds") {
      circuit c{components};

      c.step_();

      REQUIRE(c.solution("1") == Approx(1.0f));
      REQUIRE(c.solution("2") == Approx(0.5f));

      constexpr auto NITER = 44100;

      const auto start = high_resolution_clock::now();

      for(auto i = 0; i < NITER; i++)
        c.step_();

      const auto delta = high_resolution_clock::now() - start;
      const auto avgTime = nanoseconds{delta}.count()/NITER;

      INFO( "average iteration time: " << avgTime << " ns");
      REQUIRE(c.solution("1") == Approx(1.0f));
      REQUIRE(c.solution("2") == Approx(0.5f));

    }

  }

}

SCENARIO("nonlinear simulation", "[circuit]") {

  GIVEN("a nonlinear circuit (diode)") {

    vector<component::ptr> components {
      make_component<dc_current>     ("I1", "0", "1", 5.0e-3f),
      make_component<linear_resistor>("R1", "1", "2", 2.2e+3f),
      make_component<basic_diode>    ("D1", "2", "0", 4.352e-9f, 1.906f),
    };
    circuit c{components};

    THEN("Newton-Raphson converges") {
      {
        const auto start = high_resolution_clock::now();

        const auto i = c.nr_step_();

        const auto delta = high_resolution_clock::now() - start;
        const auto RunTimeDC = nanoseconds{delta}.count();


        INFO("DC Runtime =  " << RunTimeDC << " ns");
        INFO("V[diode] = " << *c.get_x("2") << " V");
        REQUIRE( i > 0 );
      }

      {
        const auto start = high_resolution_clock::now();

        for(auto i = 0; i < 100; i++) c.step_();

        const auto delta = high_resolution_clock::now() - start;
        const auto RunTimeBasicStep = nanoseconds{delta}.count();


        INFO("Average Runtime Basic Step =  " << RunTimeBasicStep/100 << " ns");
        INFO("V[diode] = " << *c.get_x("2") << " V");
        REQUIRE(true);
      }

    }

  }

}

SCENARIO("dynamic simulation", "[circuit]") {

  GIVEN("a linear dynamic circuit") {

    vector<component::ptr> components {

      make_component<dc_current>      ("I1", "0", "1", 1.0e-3f), //1uA
      make_component<linear_resistor> ("R1", "1", "2", 2.2e+3f),
      make_component<linear_capacitor>("C1", "2", "0", 10e-6), // 1uF

    };
    circuit c{components};

    constexpr float delta_t = 1.0e-6f;
    constexpr int   niter   = 10;

    THEN("time simulation converges") {
      const auto start = high_resolution_clock::now();

      const auto i = c.advance_(delta_t);

      const auto delta = high_resolution_clock::now() - start;
      const auto RunTimeTran = nanoseconds{delta}.count();

      INFO("Transient step Runtime =  " << RunTimeTran << " ns");
      INFO("V[cap] = " << *c.get_x("2") << " V");
      REQUIRE( i > 0 );
    }

    THEN("time moves forward") {

      std::vector<float> vs(niter);
      std::vector<int>   is(niter);

      const auto vptr = c.get_x("2");

      const auto start = high_resolution_clock::now();

      for(auto iter = 0; iter < niter; ++iter) {
        is[iter] = c.advance_(delta_t);
        vs[iter] = *vptr;
      }

      const auto delta = high_resolution_clock::now() - start;
      const auto RunTimeTran = nanoseconds{delta}.count();

      for(auto iter = 0; iter < niter; ++iter) {
        INFO("V[cap] = " << vs[iter] << " V");
        REQUIRE(is[iter] > 0);
      }
      INFO("Transient step Runtime =  " << RunTimeTran/niter << " ns");
      REQUIRE(true);

    }

  }

}

SCENARIO("nonlinear dynamic simulation", "[circuit]") {

  GIVEN("a nonlinear dynamic circuit") {

    vector<component::ptr> components {

      make_component<ac_voltage>      ("V1", "1", "0", 12.0f, 1.0e3, 0.0f), //12 V, 1 kHz
      make_component<basic_diode>     ("D1", "1", "2", 4.352e-9f, 1.906f),  //1n4148
      make_component<linear_resistor> ("R1", "2", "0", 2.2e+3f),            //2.2 kR
      make_component<linear_capacitor>("C1", "2", "0", 10e-6),              //1 uF

    };
    circuit c{components};

    constexpr float delta_t = 1.0e-6f;
    constexpr int   niter   = 10;

    THEN("time simulation converges") {
      const auto start = high_resolution_clock::now();

      const auto i = c.advance_(delta_t);

      const auto delta = high_resolution_clock::now() - start;
      const auto RunTimeTran = nanoseconds{delta}.count();

      INFO("Transient step Runtime =  " << RunTimeTran << " ns");
      INFO("V[cap] = " << *c.get_x("2") << " V");
      REQUIRE( i > 0 );
    }

    THEN("time moves forward") {

      std::vector<float> vs(niter);
      std::vector<int>   is(niter);

      const auto vptr = c.get_x("2");

      const auto start = high_resolution_clock::now();

      for(auto iter = 0; iter < niter; ++iter) {
        is[iter] = c.advance_(delta_t);
        vs[iter] = *vptr;
      }

      const auto delta = high_resolution_clock::now() - start;
      const auto RunTimeTran = nanoseconds{delta}.count();

      for(auto iter = 0; iter < niter; ++iter) {
        INFO("V[cap] = " << vs[iter] << " V");
        REQUIRE(is[iter] > 0);
      }
      INFO("Transient step Runtime =  " << RunTimeTran/niter << " ns");
      REQUIRE(true);

    }

    THEN("basic simulation") {

      const int sim_size = 5.0e-3f / delta_t;

      std::vector<float> vs(sim_size);
      std::vector<int>   is(sim_size);

      const auto vptr = c.get_x("2");

      const auto start = high_resolution_clock::now();

      for(auto iter = 0; iter < sim_size; ++iter) {
        is[iter] = c.advance_(delta_t);
        vs[iter] = *vptr;
      }

      const auto delta = high_resolution_clock::now() - start;
      const auto RunTimeTran = nanoseconds{delta}.count();

      REQUIRE(std::all_of(is.cbegin(), is.cend(), [](auto i){ return i > 0; }));
      INFO("Transient step Runtime =  " << RunTimeTran/sim_size << " ns");
      REQUIRE(true);

      std::ofstream v_file("sim_v.txt");
      std::copy(vs.cbegin(), vs.cend(), std::ostream_iterator<float>(v_file, "\n"));

      std::ofstream i_file("sim_i.txt");
      std::copy(is.cbegin(), is.cend(), std::ostream_iterator<int>(i_file, "\n"));

    }

  }
}

SCENARIO("basic circuit simulation", "[circuit]") {

  GIVEN("a nonlinear dynamic circuit") {

    vector<component::ptr> components {

      make_component<ac_voltage>      ("V1", "IN",    "0",     0.1f, 1e3f, 1.0f), //1.0 V, 440 Hz
      make_component<linear_resistor> ("RA", "IN",    "PLUS",  4.7e+3f),
      make_component<linear_resistor> ("R5", "PLUS",  "0",     1.0e+3f),
      make_component<ideal_opamp>     ("U1", "OUT",   "0",     "PLUS", "MINUS"),
      make_component<linear_capacitor>("C3", "MINUS", "1",     47.0e-9f),
      make_component<linear_resistor> ("R4", "1",     "0",     4.7e+3f),
      make_component<basic_diode>     ("D1", "OUT",   "MINUS", 4.352e-9f, 1.906f),
      make_component<basic_diode>     ("D2", "MINUS", "OUT",   4.352e-9f, 1.906f),
      make_component<linear_resistor> ("R6", "OUT",   "MINUS", 351.0e+3f),
      make_component<linear_resistor> ("RL", "OUT",   "0",     1.0e+3f),


    };
    circuit c{components};

    constexpr float delta_t = 1.0 / 44100.0f;
    constexpr int   niter   = 1000;

    THEN("basic simulation") {

      std::vector<float> vs(niter);
      std::vector<int>   is(niter);
      std::vector<std::chrono::time_point<high_resolution_clock>> ts(niter);

      const auto vptr = c.get_x("OUT");

      const auto start = high_resolution_clock::now();

      for(auto iter = 0; iter < niter; ++iter) {
        is[iter] = c.advance_(delta_t);
        vs[iter] = *vptr;
        ts[iter] = high_resolution_clock::now();
      }

      const auto delta = high_resolution_clock::now() - start;
      const auto RunTimeTran = nanoseconds{delta}.count();

      REQUIRE(std::all_of(is.cbegin(), is.cend(), [](auto i){ return i > 0; }));
      INFO("Average transient step runtime =  " << RunTimeTran/niter << " ns");
      REQUIRE(true);

      std::ofstream v_file("sim_vout.txt");
      std::copy(vs.cbegin(), vs.cend(), std::ostream_iterator<float>(v_file, "\n"));

      std::ofstream i_file("sim_steps.txt");
      std::copy(is.cbegin(), is.cend(), std::ostream_iterator<int>(i_file, "\n"));

      std::ofstream t_file("sim_times.txt");
      auto it = std::ostream_iterator<int>(t_file, "\n");
      for(auto i = 1; i < niter; ++i)
        *it++ = nanoseconds{ts[i] - ts[i-1]}.count();
    }

  }

}
