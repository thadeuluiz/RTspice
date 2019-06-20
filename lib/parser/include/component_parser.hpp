/*!
 *    @file  component_parser.hpp
 *   @brief basic defintions for the component parsers
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

#ifndef  component_parser_INC
#define  component_parser_INC

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include "component.hpp"

namespace rtspice::parser {

  using components::component;
  using components::make_component;

  namespace qi = boost::spirit::qi;

  struct si_prefixes : qi::symbols<char, float> {

    si_prefixes() {
      add
        ("Y", 1e+24)
        ("Z", 1e+21)
        ("E", 1e+18)
        ("P", 1e+15)
        ("T", 1e+12)
        ("G", 1e+09)
        ("M", 1e+06)
        ("k", 1e+03)
        ("h", 1e+02)
        ("da",1e+01)
        ("d", 1e-01)
        ("c", 1e-02)
        ("m", 1e-03)
        ("u", 1e-06)
        ("n", 1e-09)
        ("p", 1e-12)
        ("f", 1e-15)
        ("a", 1e-18)
        ("z", 1e-21)
        ("y", 1e-24);
    }

  };

  template<class Iterator, class Skipper>
  struct component_parser : qi::grammar<Iterator, Skipper, component::ptr()> {

    template<class Start>
    explicit component_parser(Start&& start) :
      component_parser::base_type{ std::forward<Start>(start) } {

        using namespace qi;

        id_   %= +alnum;
        value_ = float_[_val = _1] >> -prefix_[_val *= _1];
    }

    protected:
      qi::rule<Iterator, std::string()> id_;
      qi::rule<Iterator, float()>       value_;
    private:
      si_prefixes                 prefix_;
  };

}		// -----  end of namespace rtspice::parser  -----

#endif   // ----- #ifndef component_parser_INC  -----
