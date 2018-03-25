////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

#ifndef SISL_DELEGATE_HPP
#define SISL_DELEGATE_HPP

#include <memory>
#include <functional>
#include <cstring>
#include <type_traits>
#include <typeinfo>

#include "Generic_Delegate.hpp"

namespace sisl
{
  namespace priv
  {

  template< typename RET , typename... ARGS >
  class Delegate : public Generic_Delegate
  {

    private:

      template< typename OBJ >
      void init_with_member(OBJ* obj);

      void init_with_static();

    public:
      Delegate() = default;

      Delegate(const Delegate&) = default;

      Delegate(Delegate&&) = default;

      Delegate( const std::function<RET(ARGS...)>& functor );

      template< typename OBJ >
      Delegate( OBJ* obj , RET(OBJ::*fp)(ARGS...));

      Delegate( RET(*fp)(ARGS...) );

      template< typename... ARGS2 >
      inline RET call(ARGS2... args);

      template< typename... ARGS2 >
      RET operator()(ARGS2... args);

  };
}
}

#ifndef SISL_DELEGATE_TPP
#define SISL_DELEGATE_TPP
#include "../templates/Delegate.tpp"
#endif //SISL_DELEGATE_TPP

#endif //SISL_DELEGATE_HPP
