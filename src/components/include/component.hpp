/*!
 *    @file  component.hpp
 *   @brief component base class definitions
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

#ifndef  component_INC
#define  component_INC

#include <memory>
#include <string>

namespace rtspice::components {

  /*!
   *  @brief node id and offset pair
   */
  struct node {

    node() {}

    node(std::string _id, std::size_t _offset = ~std::size_t{}):
      id{std::move(_id)}, offset{_offset} {}

    std::string id;
    std::size_t offset;
  };

  /*!
   *  @brief component base class
   */
  class component {
    private:
    protected:
      component(std::string id) : id_{std::move(id)} {}
      std::string id_;
    public:

      auto& id() const noexcept { return id_; }
      using ptr = std::shared_ptr<component>;

      virtual ~component() = default;
  };

  /*!
   *  @brief component factory function object
   */
  template<class derived>
  constexpr auto make_component = [](auto&&... args) -> component::ptr {
    auto ptr =  new derived{std::forward<decltype(args)>(args)...};
    return component::ptr{ptr};
  };

}		// -----  end of namespace rtspice::components  -----

#endif   // ----- #ifndef component_INC  -----
