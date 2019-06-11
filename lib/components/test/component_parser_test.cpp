/*!
 *    @file  component_parser_test.cpp
 *   @brief tester for several component definitions
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/09/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include <catch2/catch.hpp>
#include "component_parser.hpp"

#include "resistor_parser.hpp"
#include "source_parser.hpp"

using namespace std::string_literals;
using namespace rtspice::components;

SCENARIO("resistor_parser identifies resistor statements", "[resistor_parser]") {

  parser::resistor_parser<std::string::const_iterator, parser::qi::space_type> grammar;

  GIVEN("a linear resistor statement") {

    const std::string statement = "RX net0 net1 1e2";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = parser::qi::phrase_parse(begin,
                                          end,
                                          grammar,
                                          parser::qi::space,
                                          component_);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(begin == end);
      }
      THEN("component is created") {
        REQUIRE(component_ != nullptr);
        REQUIRE(component_->id() == "RX"s);
      }

    }

  }

}

SCENARIO("voltage_source identifies voltage statements", "[voltage_source_parser]") {

  parser::voltage_source_parser<std::string::const_iterator, parser::qi::space_type> grammar;

  GIVEN("an independent voltage source statement") {

    const std::string statement = "VX net0 net1 12";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = parser::qi::phrase_parse(begin,
                                          end,
                                          grammar,
                                          parser::qi::space,
                                          component_);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(begin == end);
      }
      THEN("component is created") {
        REQUIRE(component_ != nullptr);
        REQUIRE(component_->id() == "VX"s);
      }

    }

  }

}

SCENARIO("current_source_parser identifies current statements", "[current_source_parser]") {

  parser::current_source_parser<std::string::const_iterator, parser::qi::space_type> grammar;

  GIVEN("an independent current source statement") {

    const std::string statement = "IX net0 net1 1e-3";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = parser::qi::phrase_parse(begin,
                                          end,
                                          grammar,
                                          parser::qi::space,
                                          component_);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(begin == end);
      }
      THEN("component is created") {
        REQUIRE(component_ != nullptr);
        REQUIRE(component_->id() == "IX"s);
      }

    }

  }

}
