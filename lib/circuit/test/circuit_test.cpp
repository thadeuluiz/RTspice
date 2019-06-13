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

#include <catch2/catch.hpp>


#include "circuit.hpp"
#include "resistor.hpp"
#include "sources.hpp"


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
