/*!
 *    @file  circuit_test.cpp
 *   @brief basic circuit functionality test
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/11/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include <string>
#include <chrono>
#include <numeric>

#include <fstream>
#include <iterator>

#include <catch2/catch.hpp>


#include "circuit.hpp"
#include "resistor.hpp"
#include "sources.hpp"
#include "dynamic.hpp"
#include "opamp.hpp"
#include "bipolar.hpp"


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
    }

    THEN("stepping succeeds") {
      circuit c{components};

      c.nr_step_();

      CHECK(*c.get_x("1") == Approx(1.0f));
      CHECK(*c.get_x("2") == Approx(0.5f));

      constexpr auto NITER = 44100;

      const auto start = high_resolution_clock::now();

      for(auto i = 0; i < NITER; i++)
        c.nr_step_();

      const auto delta = high_resolution_clock::now() - start;
      const auto avgTime = nanoseconds{delta}.count()/NITER;

      INFO( "average iteration time: " << avgTime << " ns");
      CHECK(*c.get_x("1") == Approx(1.0f));
      CHECK(*c.get_x("2") == Approx(0.5f));
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
        CHECK( i > 0 );
      }

      {
        const auto start = high_resolution_clock::now();

        for(auto i = 0; i < 100; i++) c.nr_step_();

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
        CHECK(is[iter] > 0);
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

  constexpr auto dist = 200.0e3;
  constexpr auto tone = 19.0e3;

  GIVEN("a distortion circuit") {

    vector<component::ptr> components {

      //clipping section
      //input
      make_component<ac_voltage>      ("V1",  "0", "3", 0.1, 10e3, 0.0), //100 mV, 440 Hz

      //opamp
      make_component<ideal_opamp>     ("U1A", "1", "0", "2", "3"),

      //feedback
      make_component<linear_resistor> ("R4",  "A", "0", 4.7e3),
      make_component<linear_capacitor>("C3",  "2", "A", 47.0e-9),

      make_component<linear_capacitor>("C4",  "1", "2", 51.0e-12),
      make_component<basic_diode>     ("D1",  "1", "2", 4.352e-9f, 1.906f),  //1n4148
      make_component<basic_diode>     ("D2",  "2", "1", 4.352e-9f, 1.906f),  //1n4148
      make_component<variable_resistor>("R6", "2", "1", 51.0e3, "dist"),

      //tone section
      make_component<linear_resistor> ("R7",  "1", "5", 4.7e3),
      make_component<linear_capacitor>("C5",  "5", "0", 0.22e-6),
      make_component<linear_resistor> ("R9",  "5", "0", 10.0e3),

      make_component<ideal_opamp>     ("U1B", "7", "0", "5", "6"),

      make_component<linear_resistor> ("R8",  "0", "B", 220.0),
      make_component<linear_capacitor>("C6",  "B", "T", 0.22e-6),
      make_component<linear_resistor> ("RTa", "5", "T", 20.0e3 - tone),
      make_component<linear_resistor> ("RTb", "T", "6", tone),

      make_component<linear_resistor> ("R11", "6", "7", 1.0e3),

    };
    circuit c{components};

    constexpr float delta_t = 1.0 / 44100.0;
    constexpr int   niter   = 44100;

    THEN("basic simulation") {

      //std::vector<float> vs(niter);
      std::vector<int>   is(niter);
      //std::vector<std::chrono::time_point<high_resolution_clock>> ts(niter);

      const auto vptr = c.get_x("7");

      //c.nr_step_(); //dc point

      const auto start = high_resolution_clock::now();

      for(auto iter = 0; iter < niter; ++iter) {
        is[iter] = c.advance_(delta_t);
        //vs[iter] = *vptr;
        //ts[iter] = high_resolution_clock::now();
      }

      const auto delta = high_resolution_clock::now() - start;
      const auto RunTimeTran = nanoseconds{delta}.count();
      CHECK(std::all_of(is.cbegin(), is.cend(), [](auto i){ return i > 0; }));

      const auto inner_it = std::accumulate(is.cbegin(), is.cend(), 0);

      INFO("Average solve runtime =  " << RunTimeTran/inner_it << " ns");
      REQUIRE(true);

      //std::ofstream v_file("sim_vout.txt");
      //std::copy(vs.cbegin(), vs.cend(), std::ostream_iterator<float>(v_file, "\n"));

      //std::ofstream i_file("sim_steps.txt");
      //std::copy(is.cbegin(), is.cend(), std::ostream_iterator<int>(i_file, "\n"));

      //std::ofstream t_file("sim_times.txt");
      //auto it = std::ostream_iterator<int>(t_file, "\n");
      //for(auto i = 1; i < niter; ++i)
      //  *it++ = nanoseconds{ts[i] - ts[i-1]}.count();
    }

  }

}

SCENARIO("basic transistor simulation", "[bipolar_npn]") {
  constexpr auto niter = 1024;
  constexpr auto delta_t = 1e-5;

  GIVEN("a common emmiter circuit") {

    vector<component::ptr> components {
      make_component<dc_voltage>      ("VCC", "VCC", "0", 9),
      make_component<ac_voltage>      ("VIN", "0", "1", 100e-3, 1e3, 0.0f),
      make_component<linear_capacitor>("CB",  "1", "B", 1e-6),
      make_component<linear_resistor> ("R1", "VCC","B", 4.7e3),
      make_component<linear_resistor> ("R2",  "B", "0", 1e3),
      make_component<linear_resistor> ("RC", "VCC", "C", 4.7e3),
      make_component<linear_resistor> ("RE", "E", "0", 1e3),
      make_component<bipolar_npn>     ("Q1", "C", "B", "E", 3.83e-14, 324.4, 8.29),
      make_component<linear_capacitor>("CE", "E", "0", 20e-6),
      make_component<linear_capacitor>("CC", "C", "OUT", 1e-6),
      make_component<linear_resistor> ("RL", "OUT", "0", 100e3),
    };
    circuit c{components};

    THEN("simulation works") {

      std::vector<float> vs;

      //DC point simulation
      for(auto i = 0; i < 100*niter; ++i) {
        c.advance_(delta_t);
      }

      for(auto i = 0; i < niter; ++i) {
        c.advance_(delta_t);
        vs.push_back(*c.get_x("OUT"));
      }

      std::ofstream v_file("qsim_vout.txt");
      std::copy(vs.cbegin(), vs.cend(), std::ostream_iterator<float>(v_file, "\n"));

    }

  }
}
