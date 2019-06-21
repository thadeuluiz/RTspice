/*!
 *    @file  netlist_parser.hpp
 *   @brief Circuit Netlist parser
 *
 *  <+DETAILED+>
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/06/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */


#ifndef  netlist_parser_INC
#define  netlist_parser_INC

#include "resistor_parser.hpp"
#include "source_parser.hpp"
#include "dynamic_parser.hpp"
#include "opamp_parser.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>


namespace rtspice::parser {

  namespace qi = boost::spirit::qi;

  /*!
   *  @brief qi grammar for spice netlist statements
   *
   *  netlist statement grammar, outputs statements as strings to an std::vector
   *
   * @tparam Iterator the sequence iterator type
   * @tparam Container an stl (or compatible) container template
   */
  template<class Iterator, class Skipper>
  struct line_parser : qi::grammar<Iterator, Skipper, std::vector<std::string>()> {

    line_parser() : line_parser::base_type{start_} {

      using namespace qi;

      start_      %= *(comment_ | line_);
      comment_    = '*' >> *(char_ - eol) >> eol;
      basic_line_ %= *(char_ - eol) >> eol;
      line_       =  (basic_line_[_val += _1] >> *comment_) % ('+' >> eps[_val += " "]);

    }

    private:
      qi::rule<Iterator, Skipper, std::vector<std::string>()> start_;
      qi::rule<Iterator, qi::unused_type()>          comment_;
      qi::rule<Iterator, std::string()>              basic_line_;
      qi::rule<Iterator, Skipper, std::string()>     line_;

  };

  /*!
   * @brief combined parser for all known components
   */
  template<class Iterator, class Skipper>
  struct statement_parser : component_parser<Iterator, Skipper> {

    statement_parser() :
      component_parser<Iterator, Skipper>{ start_ } {

        start_ %= resistor_
          | source_
          | capacitor_
          | inductor_
          | diode_
          | opamp_;

    }

    private:
      qi::rule<Iterator, Skipper, components::component::ptr()> start_;

      resistor_parser      <Iterator, Skipper>   resistor_;
      diode_parser         <Iterator, Skipper>   diode_;
      source_parser        <Iterator, Skipper>   source_;
      capacitor_parser     <Iterator, Skipper>   capacitor_;
      inductor_parser      <Iterator, Skipper>   inductor_;
      opamp_parser         <Iterator, Skipper>   opamp_;
  };


}		// -----  end of namespace rtspice::parser  -----

#endif   // ----- #ifndef netlist_parser_INC  -----
