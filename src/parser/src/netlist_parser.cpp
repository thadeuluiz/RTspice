/*!
 *    @file  netlist_parser.cpp
 *   @brief  
 *
 *  <+DETAILED+>
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd), thadeuluiz@poli.ufrj.br
 *
 *  @internal
 *       Created:  06/07/2019
 *      Revision:  none
 *      Compiler:  gcc
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

//#include "netlist_parser.hpp"

#include <boost/spirit/home/x3.hpp>

namespace x3 = boost::spirit::x3;

struct si_prefixes_ : x3::symbols<double> {

  si_prefixes_() {
    add
      ("y", 1e-24)
      ("z", 1e-21)
      ("a", 1e-18)
      ("f", 1e-15)
      ("p", 1e-12)
      ("n", 1e-09)
      ("u", 1e-06)
      ("m", 1e-03)
      ("c", 1e-02)
      ("d", 1e-01)
      ("da",1e+01)
      ("h", 1e+02)
      ("k", 1e+03)
      ("M", 1e+06)
      ("G", 1e+09)
      ("T", 1e+12)
      ("P", 1e+15)
      ("E", 1e+18)
      ("Z", 1e+21)
      ("Y", 1e+24);
  }
} si_prefix;

using x3::eps;
using x3::_val;
using x3::_attr;
using x3::double_;
using x3::char_;

const x3::rule<class real_value, double> real_value = "real_value";
const auto real_value_def =
  double_        [([](auto& ctx){ _val(ctx)  = _attr(ctx); })] //initialize
    >> -si_prefix[([](auto& ctx){ _val(ctx) *= _attr(ctx); })] //rescale
    >> -char_;                                                 //might have unit?

BOOST_SPIRIT_DEFINE(real_value);
