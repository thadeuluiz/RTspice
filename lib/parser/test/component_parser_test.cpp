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
//#include "component_parser.hpp"

#include "resistor_parser.hpp"
#include "source_parser.hpp"
#include "dynamic_parser.hpp"
#include "opamp_parser.hpp"

using namespace std;
using namespace std::string_literals;

using namespace rtspice::parser;
using namespace rtspice::components;

extern unordered_map<string, atomic<float>> gExtParams;

unordered_map<string, atomic<float>> gExtParams;

SCENARIO("resistor_parser identifies resistor statements", "[resistor_parser]") {

  resistor_parser<string::const_iterator, qi::space_type> grammar;

  GIVEN("a linear resistor statement") {

    const string statement = "RX net0 net1 1e2k";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = qi::phrase_parse(begin,
                                 end,
                                 grammar,
                                 qi::space,
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

SCENARIO("diode_parser identifies diode statements", "[diode_parser]") {

  diode_parser<string::const_iterator, qi::space_type> grammar;

  GIVEN("a diode statement") {

    const string statement = "DX net0 net1 IS=1e-9 N=1.94";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = qi::phrase_parse(begin,
                                 end,
                                 grammar,
                                 qi::space,
                                 component_);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(begin == end);
      }
      THEN("component is created") {
        REQUIRE(component_ != nullptr);
        REQUIRE(component_->id() == "DX"s);
      }
    }
  }
}

SCENARIO("voltage_source identifies voltage statements", "[voltage_source_parser]") {

  source_parser<string::const_iterator, qi::space_type> grammar;

  GIVEN("an independent DC voltage source statement") {

    const string statement = "VX net0 net1 DC 12";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = qi::phrase_parse(begin,
          end,
          grammar,
          qi::space,
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

  GIVEN("an independent AC voltage source statement") {

    const string statement = "VX net0 net1 AC 12 1e3 0";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = qi::phrase_parse(begin,
          end,
          grammar,
          qi::space,
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

  GIVEN("an independent external voltage source statement") {

    const string statement = "VX net0 net1 EXT IN";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = qi::phrase_parse(begin,
          end,
          grammar,
          qi::space,
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

SCENARIO("dynamic_parser identifies capacitor statements", "[dynamic_parser]") {

  capacitor_parser<string::const_iterator, qi::space_type> grammar;

  GIVEN("a linear capacitor statement") {

    const string statement = "CX net0 net1 4.7e-9";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = qi::phrase_parse(begin,
                                 end,
                                 grammar,
                                 qi::space,
                                 component_);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(begin == end);
      }
      THEN("component is created") {
        REQUIRE(component_ != nullptr);
        REQUIRE(component_->id() == "CX"s);
      }
    }
  }

}

SCENARIO("opamp_parser identifies opamp statements", "[opamp_parser]") {

  opamp_parser<string::const_iterator, qi::space_type> grammar;

  GIVEN("an opamp statement") {

    const string statement = "UX net0 net1 net2 net3";

    WHEN("parsed") {

      component::ptr component_;

      auto begin = statement.cbegin();
      auto end   = statement.cend();

      auto ok = qi::phrase_parse(begin,
                                 end,
                                 grammar,
                                 qi::space,
                                 component_);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(begin == end);
      }
      THEN("component is created") {
        REQUIRE(component_ != nullptr);
        REQUIRE(component_->id() == "UX"s);
      }
    }
  }
}

