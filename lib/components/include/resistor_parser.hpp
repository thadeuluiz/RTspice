/*!
 *    @file  resistor_parser.hpp
 *   @brief grammar for resistors
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/09/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */


#ifndef  resistor_parser_INC
#define  resistor_parser_INC

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

#include "component_parser.hpp"
#include "resistor.hpp"

namespace rtspice::components::parser {

  namespace qi = boost::spirit::qi;

  template<class Iterator, class Skipper>
  struct resistor_parser : component_parser<Iterator, Skipper> {

    resistor_parser() : resistor_parser::base_type{start_} {

      using qi::alnum;
      using qi::lit;
      using qi::double_;

      using qi::_val;
      using qi::_1;
      using qi::_2;
      using qi::_3;
      using qi::_4;

      using boost::phoenix::bind;

      id_ %= +alnum;

      linear_resistor_ = (id_ >> id_ >> id_ >> double_)
        [_val = bind(make_component<linear_resistor>, _1, _2, _3, _4)];

      start_ %=  &lit('R') >> linear_resistor_;
    };

    private:
      qi::rule<Iterator, std::string()>             id_;
      qi::rule<Iterator, Skipper, component::ptr()> linear_resistor_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;
  };

}		// -----  end of namespace rtspice::components::parser  -----

#endif   // ----- #ifndef resistor_parser_INC  -----
