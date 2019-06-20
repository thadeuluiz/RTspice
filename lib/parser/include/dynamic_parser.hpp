/*!
 *    @file  dynamic_parser.hpp
 *   @brief  dynamic component parser
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

#ifndef  dynamic_parser_INC
#define  dynamic_parser_INC

#include "component_parser.hpp"
#include <boost/spirit/include/phoenix_bind.hpp>

#include "dynamic.hpp"

namespace rtspice::parser {

  template<char Prefix, class F, class Iterator, class Skipper>
  struct dynamic_parser : component_parser<Iterator, Skipper> {

    using component_parser<Iterator, Skipper>::id_;
    using component_parser<Iterator, Skipper>::value_;

    dynamic_parser() : component_parser<Iterator, Skipper>{start_} {

      using namespace qi;
      using boost::phoenix::bind;

      dynamic_ = (id_ >> id_ >> id_ >> value_)[
        _val = bind(make_component<components::dynamic<F>>, _1, _2, _3, _4)];

      start_ %=  &lit(Prefix) >> dynamic_;
    };

    private:
      qi::rule<Iterator, Skipper, component::ptr()> dynamic_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;
  };

  template<class Iterator, class Skipper>
  using capacitor_parser = dynamic_parser<'C', components::linear_capacitor_trapezoidal, Iterator, Skipper>;

  template<class Iterator, class Skipper>
  using inductor_parser = dynamic_parser<'L', components::linear_inductor_trapezoidal, Iterator, Skipper>;

}		// -----  end of namespace rtspice::parser  -----

#endif   // ----- #ifndef dynamic_parser_INC  -----
