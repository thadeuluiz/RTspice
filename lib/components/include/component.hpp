/*!
 *    @file  component.hpp
 *   @brief component base class definitions
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

#ifndef  component_INC
#define  component_INC

#include <memory>
#include <string>

namespace rtspice::circuit {
  class circuit;
}

namespace rtspice::components {

  /*!
   *  @brief component base class
   */
  class component {
    private:

    protected:
      component(std::string id) : id_{std::move(id)} {}
      std::string id_;
    public:

      //enables optimization for matrix filling, by preserving static information
      virtual bool is_static()    const = 0;
      virtual bool is_dynamic()   const = 0;
      virtual bool is_nonlinear() const = 0;

      //registers the names of all needed variables to the system
      virtual void register_(circuit::circuit& circuit) = 0;

      //recover the address of system entries
      virtual void setup(circuit::circuit& circuit) = 0;

      virtual void fill() = 0;

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
