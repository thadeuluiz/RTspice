/*!
 *    @file  source_parser.hpp
 *   @brief  
 *
 *  <+DETAILED+>
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

#ifndef  source_parser_INC
#define  source_parser_INC

#include "component_parser.hpp"
#include "sources.hpp"

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace rtspice::components::parser {

  namespace qi = boost::spirit::qi;

  template<class Iterator, class Skipper>
  struct voltage_source_parser : component_parser<Iterator, Skipper> {

    voltage_source_parser() : voltage_source_parser::base_type{start_} {

      using qi::alnum;
      using qi::lit;
      using qi::float_;

      using qi::_val;
      using qi::_1;
      using qi::_2;
      using qi::_3;
      using qi::_4;

      using boost::phoenix::bind;

      id_ %= +alnum;

      independent_source_ = (id_ >> id_ >> id_ >> float_)
        [_val = bind(make_component<dc_voltage>, _1, _2, _3, _4)];

      start_ %=  &lit('V') >> independent_source_;
    };

    private:
      qi::rule<Iterator, std::string()>             id_;
      qi::rule<Iterator, Skipper, component::ptr()> independent_source_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;

  };

  template<class Iterator, class Skipper>
  struct current_source_parser : component_parser<Iterator, Skipper> {

    current_source_parser() : current_source_parser::base_type{start_} {
      using qi::alnum;
      using qi::lit;
      using qi::float_;

      using qi::_val;
      using qi::_1;
      using qi::_2;
      using qi::_3;
      using qi::_4;

      using boost::phoenix::bind;

      id_ %= +alnum;

      independent_source_ = (id_ >> id_ >> id_ >> float_)
        [_val = bind(make_component<dc_current>, _1, _2, _3, _4)];

      start_ %=  &lit('I') >> independent_source_;
    };

    private:
      qi::rule<Iterator, std::string()>             id_;
      qi::rule<Iterator, Skipper, component::ptr()> independent_source_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;

  };

}		// -----  end of namespace rtspice::components::parser  -----
#endif   // ----- #ifndef source_parser_INC  ----- 
