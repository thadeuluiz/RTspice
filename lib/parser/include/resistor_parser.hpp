/*!
 *    @file  resistor_parser.hpp
 *   @brief grammar for resistors
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

#ifndef  resistor_parser_INC
#define  resistor_parser_INC

#include "component_parser.hpp"

#include <boost/spirit/include/phoenix_bind.hpp>

#include "resistor.hpp"

namespace rtspice::parser {

  namespace qi = boost::spirit::qi;

  template<class Iterator, class Skipper>
  struct resistor_parser : component_parser<Iterator, Skipper> {

    resistor_parser() : component_parser<Iterator, Skipper>{start_} {

      using namespace qi;
      using boost::phoenix::bind;

      linear_resistor_ = (id_ >> id_ >> id_ >> value_)[
        _val = bind(make_component<components::linear_resistor>, _1, _2, _3, _4)];

      variable_resistor_ = (id_ >> id_ >> id_ >> lit("EXT") >> value_ >> id_)[
        _val = bind(make_component<components::variable_resistor>, _1, _2, _3, _4, _5)];

      start_ %=  &lit('R') >> linear_resistor_ | variable_resistor_;

    };

    private:
      using component_parser<Iterator, Skipper>::id_;
      using component_parser<Iterator, Skipper>::value_;

      qi::rule<Iterator, Skipper, component::ptr()> linear_resistor_;
      qi::rule<Iterator, Skipper, component::ptr()> variable_resistor_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;
  };

  template<class Iterator, class Skipper>
  struct diode_parser : component_parser<Iterator, Skipper> {

    diode_parser() : component_parser<Iterator, Skipper>{start_} {

      using namespace qi;
      using boost::phoenix::bind;

      basic_diode_ = (id_ >> id_ >> id_>> lit("IS=") >> value_ >> lit("N=") >> value_)[
        _val = bind(make_component<components::basic_diode>, _1, _2, _3, _4, _5)];

      start_ %=  &lit('D') >> basic_diode_;

    };

    private:
      using component_parser<Iterator, Skipper>::id_;
      using component_parser<Iterator, Skipper>::value_;

      qi::rule<Iterator, Skipper, component::ptr()> basic_diode_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;
  };

}		// -----  end of namespace rtspice::components::parser  -----

#endif   // ----- #ifndef resistor_parser_INC  -----
