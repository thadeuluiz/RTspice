/*!
 *    @file  probe_parser.hpp
 *   @brief  syntax for variable probing
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/22/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#ifndef  probe_parser_INC
#define  probe_parser_INC

#include "component_parser.hpp"

#include <boost/spirit/include/phoenix_bind.hpp>

#include "probe.hpp"


namespace rtspice::parser {

  template<class Iterator, class Skipper>
  struct probe_parser : component_parser<Iterator, Skipper> {

    probe_parser() : component_parser<Iterator, Skipper>{start_} {

      using namespace qi;
      using boost::phoenix::bind;

      start_ =  lit("PROBE") >> id_[
        _val = bind(make_component<components::probe>, _1)];

    };

    private:
      using component_parser<Iterator, Skipper>::id_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;
  };

}		// -----  end of namespace rtspice::parser  -----

#endif   // ----- #ifndef probe_parser_INC  -----
