/*!
 *    @file  netlist_parser.hpp
 *   @brief Circuit Netlist parser
 *
 *  <+DETAILED+>
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/06/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */


#ifndef  netlist_parser_INC
#define  netlist_parser_INC

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>


namespace spice::parser {

  namespace qi = boost::spirit::qi;


  // Split file into lines, joining possibly multiline commands and skipping
  // comments
  
  template<class Iterator>
  struct line_parser : qi::grammar<Iterator, std::vector<std::string>()> {

    line_parser() : line_parser::base_type{start_} {

      using qi::char_;
      using qi::eol;
      using qi::eps;
      using qi::_a;
      using qi::_1;
      using qi::_val;


      comment_    = '*' >> *(char_ - eol) >> eol; //skip commented lines
      basic_line_ %= *(char_ - eol) >> eol; //
      line_       =  (basic_line_[_val += _1] >> *comment_) 
        % ('+' >> eps[_val += " "]);
      start_      %= *line_;
    }

    qi::rule<Iterator, qi::unused_type()>          comment_;
    qi::rule<Iterator, std::string()>              basic_line_;
    qi::rule<Iterator, std::string()>              line_;
    qi::rule<Iterator, std::vector<std::string>()> start_;

  };


}		// -----  end of namespace spice::parser  -----


#endif   // ----- #ifndef netlist_parser_INC  -----
