/*!
 *    @file  line_parser_test.cpp
 *   @brief line parser tests
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/08/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#include <string>
#include <vector>

#include <catch2/catch.hpp>
#include "netlist_parser.hpp"

using namespace std::string_literals;

namespace parser = rtspice::parser;

SCENARIO("line_parser identifies statements", "[line_parser]") {

  parser::line_parser<std::string::const_iterator, parser::qi::space_type> grammar;
  auto space = parser::qi::space;

  GIVEN("a netlist with a single comment") {

    const std::string netlist = "*a comment line.\n";

    WHEN("parsed") {

      std::vector<std::string> statements;

      auto start = netlist.cbegin();
      auto end   = netlist.cend();

      auto ok = parser::qi::phrase_parse(start,
          end,
          grammar,
          space,
          statements);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(start == end);
      }
      THEN("no statements are added") {
        REQUIRE(statements.size() == 0);
      }

    }

  }

  GIVEN("a netlist with multiple comments") {

    const std::string netlist = "*a comment line.\n\t*and another one.\n";

    WHEN("parsed") {

      std::vector<std::string> statements;

      auto start = netlist.cbegin();
      auto end   = netlist.cend();

      auto ok = parser::qi::phrase_parse(start,
          end,
          grammar,
          space,
          statements);
      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(start == end);
      }
      THEN("no statements are added") {
        REQUIRE(statements.size() == 0);
      }

    }

  }

  GIVEN("a netlist with a single-line statement") {

    //with white spaces
    const std::string netlist = "    a single-line statement.\n";

    WHEN("parsed") {

      std::vector<std::string> statements;

      auto start = netlist.cbegin();
      auto end   = netlist.cend();

      auto ok = parser::qi::phrase_parse(start,
          end,
          grammar,
          space,
          statements);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(start == end);
      }
      THEN("a statement is added") {
        REQUIRE(statements.size() == 1);
        REQUIRE(statements.front() == "a single-line statement."s);
      }

    }

  }

  GIVEN("a netlist with a multi-line statement") {

    //spacing before +
    const std::string netlist = "a multi-line\n  +statement.\n";

    WHEN("parsed") {

      std::vector<std::string> statements;

      auto start = netlist.cbegin();
      auto end   = netlist.cend();

      auto ok = parser::qi::phrase_parse(start,
          end,
          grammar,
          space,
          statements);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(start == end);
      }
      THEN("a statement is added") {
        REQUIRE(statements.size() == 1);
        REQUIRE(statements.front() == "a multi-line statement."s);
      }

    }

  }

  GIVEN("a netlist with a interspersed statement") {

    const std::string netlist = "a multi-line\n\t*but interspersed\n\t+statement.\n";

    WHEN("parsed") {

      std::vector<std::string> statements;

      auto start = netlist.cbegin();
      auto end   = netlist.cend();

      auto ok = parser::qi::phrase_parse(start,
          end,
          grammar,
          space,
          statements);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(start == end);
      }
      THEN("a statement is added") {
        REQUIRE(statements.size() == 1);
        REQUIRE(statements.front() == "a multi-line statement."s);
      }

    }

  }

  GIVEN("a netlist with mixed statements") {

    const std::string netlist = "a statement.\n*a comment.\n";

    WHEN("parsed") {

      std::vector<std::string> statements;

      auto start = netlist.cbegin();
      auto end   = netlist.cend();

      auto ok = parser::qi::phrase_parse(start,
          end,
          grammar,
          space,
          statements);

      THEN("parsing is successful") {
        REQUIRE(ok == true);
        REQUIRE(start == end);
      }
      THEN("a statement is added") {
        REQUIRE(statements.size() == 1);
        REQUIRE(statements.front() == "a statement."s);
      }

    }

  }

}
