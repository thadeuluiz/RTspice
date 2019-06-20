/*!
 *    @file  opamp_parser.hpp
 *   @brief basic opamp parser 
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/20/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */


#ifndef  opamp_parser_INC
#define  opamp_parser_INC

#include "component_parser.hpp"

#include <boost/spirit/include/phoenix_bind.hpp>

#include "opamp.hpp"

namespace rtspice::parser {

  template<class Iterator, class Skipper>
  struct opamp_parser : component_parser<Iterator, Skipper> {

    using component_parser<Iterator, Skipper>::id_;

    opamp_parser() : component_parser<Iterator, Skipper>{start_} {

      using namespace qi;
      using boost::phoenix::bind;

      ideal_opamp_ = (id_ >> id_ >> id_ >> id_ >> id_)[
        _val = bind(make_component<components::ideal_opamp>, _1, _2, _3, _4, _5)];

      start_ %=  &lit('U') >> ideal_opamp_;
    };

    private:
      qi::rule<Iterator, Skipper, component::ptr()> ideal_opamp_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;
  };

}		// -----  end of namespace rtspice::parser  -----

#endif   // ----- #ifndef opamp_parser_INC  -----
