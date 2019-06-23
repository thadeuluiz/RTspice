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

#include "netlist_parser.hpp"

using namespace std;
using namespace std::string_literals;

using namespace rtspice::parser;
using namespace rtspice::components;

statement_parser<string::const_iterator, qi::space_type> grammar;

SCENARIO("resistor parsing", "[statement_parser]") {

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
        REQUIRE(dynamic_pointer_cast<linear_resistor>(component_) != nullptr);
      }
    }
  }
}

SCENARIO("diode parsing", "[statement_parser]") {

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
        REQUIRE(dynamic_pointer_cast<basic_diode>(component_) != nullptr);
      }
    }
  }
}

SCENARIO("voltage source parsing", "[statement_parser]") {

  GIVEN("an independent DC source statement") {

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
        REQUIRE(dynamic_pointer_cast<dc_voltage>(component_) != nullptr);
      }
    }
  }

  GIVEN("an independent AC source statement") {

    const string statement = "VX net0 net1 SINE 12 1e3 0";

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
        REQUIRE(dynamic_pointer_cast<ac_voltage>(component_) != nullptr);
      }
    }
  }

  GIVEN("an independent external source statement") {

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
        REQUIRE(dynamic_pointer_cast<ext_voltage>(component_) != nullptr);
      }
    }
  }

}

SCENARIO("current source parsing", "[statement_parser]") {

  GIVEN("an independent DC source statement") {

    const string statement = "IX net0 net1 DC 12";

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
        REQUIRE(component_->id() == "IX"s);
        REQUIRE(dynamic_pointer_cast<dc_current>(component_) != nullptr);
      }
    }
  }

  GIVEN("an independent AC source statement") {

    const string statement = "IX net0 net1 SINE 12 1e3 0";

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
        REQUIRE(component_->id() == "IX"s);
        REQUIRE(dynamic_pointer_cast<ac_current>(component_) != nullptr);
      }
    }
  }

  GIVEN("an independent external source statement") {

    const string statement = "IX net0 net1 EXT IN";

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
        REQUIRE(component_->id() == "IX"s);
        REQUIRE(dynamic_pointer_cast<ext_current>(component_) != nullptr);
      }
    }
  }
}

SCENARIO("dynamic component parser", "[statement_parser]") {

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
        REQUIRE(dynamic_pointer_cast<linear_capacitor>(component_) != nullptr);
      }
    }
  }

}

SCENARIO("OPAMP parsing", "[statement_parser]") {

  GIVEN("an OPAMP statement") {

    const string statement = "UX net0 net1 net2 net3 OPAMP";

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
        REQUIRE(dynamic_pointer_cast<ideal_opamp>(component_) != nullptr);
      }
    }
  }
}

