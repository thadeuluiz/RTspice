/*!
 *    @file  source_parser.hpp
 *   @brief  parsers related to voltage and current sources
 *
 *  @author  Thadeu Luiz Barbosa Dias (tlbd)
 *
 *  @internal
 *       Created:  06/09/2019
 *      Revision:  none
 *      Compiler:  g++
 *  Organization:  SMT - Signals, Multimedia and Telecommunications Lab
 *     Copyright:  Copyright (c) 2019, Thadeu Luiz Barbosa Dias
 *
 *  This source code is released for free distribution under the terms of the
 *  GNU General Public License as published by the Free Software Foundation.
 */

#ifndef  source_parser_INC
#define  source_parser_INC

#include "component_parser.hpp"
#include <boost/spirit/include/phoenix_bind.hpp>

#include "sources.hpp"

namespace rtspice::parser {

  namespace qi = boost::spirit::qi;

  /*!
   * @brief generic parser for independent sources
   *
   * Relevant template parameters are the prefix and the source type template,
   * i.e. voltage_source<...> or current_source<...>
   */
  template<char Prefix, template<class...> class Source, class Iterator, class Skipper>
  struct independent_source_parser : component_parser<Iterator, Skipper> {

    using component_parser<Iterator, Skipper>::id_;
    using component_parser<Iterator, Skipper>::value_;

    independent_source_parser() : component_parser<Iterator, Skipper>{start_} {

      using namespace qi;
      using namespace components;
      using boost::phoenix::bind;

      dc_source_ = (id_ >> id_ >> id_ >> lit("DC") >> value_)
        [_val = bind(make_component<Source<constant_function>>, _1, _2, _3, _4)];

      ac_source_ = (id_ >> id_ >> id_ >> lit("AC") >> value_ >> value_ >> value_)
        [_val = bind(make_component<Source<sine_function>>, _1, _2, _3, _4, _5, _6)];

      ext_source_ = (id_ >> id_ >> id_ >> lit("EXT") >> id_)
        [_val = bind(make_component<Source<external_function>>, _1, _2, _3, _4)];

      start_ %=  &lit(Prefix) >> dc_source_ | ac_source_ | ext_source_;
    };

    private:
      qi::rule<Iterator, Skipper, component::ptr()> dc_source_;
      qi::rule<Iterator, Skipper, component::ptr()> ac_source_;
      qi::rule<Iterator, Skipper, component::ptr()> ext_source_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;
  };

  /*!
   * @brief generic parser for dependent sources
   *
   * Relevant template parameters are the prefix and the source type template,
   * i.e. vcvs<...>, vccs<...>, etc...
   */
  template<char Prefix, template<class...> class Source, class Iterator, class Skipper>
  struct controlled_source_parser : component_parser<Iterator, Skipper> {

    using component_parser<Iterator, Skipper>::id_;
    using component_parser<Iterator, Skipper>::value_;

    controlled_source_parser() : component_parser<Iterator, Skipper>{start_} {

      using namespace qi;
      using namespace components;
      using boost::phoenix::bind;

      linear_ = (id_ >> id_ >> id_ >> id_ >> id_ >> value_)
        [_val = bind(make_component<Source<linear_transfer>>, _1, _2, _3, _4, _5, _6)];

      start_ %=  &lit(Prefix) >> linear_;
    };

    private:
      qi::rule<Iterator, Skipper, component::ptr()> linear_;
      qi::rule<Iterator, Skipper, component::ptr()> start_;
  };

  template<class Iterator, class Skipper>
  struct source_parser : component_parser<Iterator, Skipper> {

    source_parser() :
      component_parser<Iterator, Skipper>{ start_ } {

        start_ %= voltage_
          | current_
          | voltage_amp_
          | current_amp_
          | transconductor_
          | transresistor_;

    }

    private:
      qi::rule<Iterator, Skipper, components::component::ptr()> start_;

      independent_source_parser<'V', components::voltage_source, Iterator, Skipper> voltage_;
      independent_source_parser<'I', components::current_source, Iterator, Skipper> current_;

      controlled_source_parser<'E', components::vcvs, Iterator, Skipper> voltage_amp_;
      controlled_source_parser<'F', components::cccs, Iterator, Skipper> current_amp_;
      controlled_source_parser<'G', components::vccs, Iterator, Skipper> transconductor_;
      controlled_source_parser<'H', components::ccvs, Iterator, Skipper> transresistor_;

  };

}		// -----  end of namespace rtspice::parser  -----

#endif   // ----- #ifndef source_parser_INC  -----
