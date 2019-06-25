/*!
 *    @file  bipolar_parser.hpp
 *   @brief  bipolar transistor parser
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/25/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */


#ifndef  bipolar_parser_INC
#define  bipolar_parser_INC

#include "component_parser.hpp"
#include <boost/spirit/include/phoenix_bind.hpp>

#include "bipolar.hpp"


namespace rtspice::parser {

  namespace qi = boost::spirit::qi;

  template<class Iterator, class Skipper>
  struct bipolar_parser : component_parser<Iterator, Skipper> {

    bipolar_parser() : component_parser<Iterator, Skipper>{ start_ } {
      using namespace qi;

      npn_ = (id_ //name
          >> id_  //collector
          >> id_  //base
          >> id_  //emmiter
          >> lit("NPN")
          >> lit("IS=") >> value_
          >> lit("BF=") >> value_
          >> lit("BR=") >> value_)[
        _val = bind(make_component<components::bipolar_npn>, _1, _2, _3, _4, _5, _6, _7)];

      pnp_ = (id_
          >> id_
          >> id_
          >> id_
          >> lit("PNP")
          >> lit("IS=") >> value_
          >> lit("BF=") >> value_
          >> lit("BR=") >> value_)[
        _val = bind(make_component<components::bipolar_pnp>, _1, _2, _3, _4, _5, _6, _7)];

      start_ %= &lit('Q') >> (npn_ | pnp_);
    }

    private:
      using component_parser<Iterator, Skipper>::id_;
      using component_parser<Iterator, Skipper>::value_;
      qi::rule<Iterator, Skipper, components::component::ptr()> start_;
      qi::rule<Iterator, Skipper, component::ptr()> npn_;
      qi::rule<Iterator, Skipper, component::ptr()> pnp_;

  };


}		// -----  end of namespace rtspice::parser  -----

#endif   // ----- #ifndef bipolar_parser_INC  -----
