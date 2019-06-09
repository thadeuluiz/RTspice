/*!
 *    @file  line_parser.hpp
 *   @brief parses SPICE netlist into single line statements
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

#ifndef  line_parser_INC
#define  line_parser_INC

#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace rtspice::parser {

  namespace qi = boost::spirit::qi;

  /*!
   *  @brief qi grammar for spice netlist statements
   *
   *  netlist statement grammar, outputs statements as strings onto any stl
   *  container
   *
   * @tparam Iterator the sequence iterator type
   * @tparam Container an stl (or compatible) container template
   */
  template<class Iterator, class Skipper>
  struct line_parser : qi::grammar<Iterator, Skipper, std::vector<std::string>()> {

    line_parser() : line_parser::base_type{start_} {

      using qi::char_;
      using qi::eol;
      using qi::eps;
      using qi::_a;
      using qi::_1;
      using qi::_val;

      start_      %= *(comment_ | line_);
      comment_    = '*' >> *(char_ - eol) >> eol;
      basic_line_ %= *(char_ - eol) >> eol;
      line_       =  (basic_line_[_val += _1] >> *comment_)
											% ('+' >> eps[_val += " "]);

    }

    private:
      qi::rule<Iterator, Skipper, std::vector<std::string>()> start_;
      qi::rule<Iterator, qi::unused_type()>          comment_;
      qi::rule<Iterator, std::string()>              basic_line_;
      qi::rule<Iterator, Skipper, std::string()>              line_;

  };

}		// -----  end of namespace rtspice::parser  -----

#endif   // ----- #ifndef line_parser_INC  -----
