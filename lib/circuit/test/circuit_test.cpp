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
#include "source.hpp"


using namespace std::string_literals;
using namespace rtspice::components;

using std::vector;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

using rtspice::circuit::circuit;

SCENARIO("circuit initialization", "[circuit]") {

  GIVEN("an component initialization vector") {
    vector<component::ptr> components {
      make_component<linear_resistor>("R1", "1", "2", 1.0),
      make_component<current_source> ("I1", "0", "1", 1.0),
      make_component<linear_resistor>("R2", "2", "0", 1.0)
    };

    THEN("initialization succeeds"){
      circuit c{components};

      auto&& nodes = c.nodes();

      REQUIRE(nodes.find("0") == nodes.end());
      REQUIRE(nodes.find("1") != nodes.end());
      REQUIRE(nodes.find("2") != nodes.end());
      REQUIRE(nodes.size()    == 2);
      REQUIRE(nodes.at("1")   == 0);
      REQUIRE(nodes.at("2")   == 1);

    }

    THEN("stepping succeeds"){
      circuit c{components};

      constexpr auto NITER = 10000;

      c.step_();

      const auto start = high_resolution_clock::now();
      
      for(auto i = 0; i < NITER; i++) c.step_();

      const auto delta = high_resolution_clock::now() - start;
      const auto avgTime = duration_cast<microseconds>(delta).count()/NITER;
      INFO( "average iteration time: " << avgTime << " us");

      REQUIRE(c.solution("1") == Approx(2.0));
      REQUIRE(c.solution("2") == Approx(1.0));

      c.step_();

      REQUIRE(c.solution("1") == Approx(2.0));
      REQUIRE(c.solution("2") == Approx(1.0));

    }
  }



}
