/*!
 *    @file  component_parser.hpp
 *   @brief basic defintions for the component parsers
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/08/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#ifndef  component_parser_INC
#define  component_parser_INC

#include <memory>
#include <boost/spirit/include/qi.hpp>

#include "component.hpp"

namespace rtspice::components::parser {

  namespace qi = boost::spirit::qi;

  template<class Iterator, class Skipper>
  using component_parser = qi::grammar<Iterator, Skipper, component::ptr()>;


}		// -----  end of namespace rtspice::components::parser  -----

#endif   // ----- #ifndef component_parser_INC  ----- 
