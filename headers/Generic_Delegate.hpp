////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

#ifndef SISL_GENERIC_DELEGATE_HPP
#define SISL_GENERIC_DELEGATE_HPP

#include <memory>
#include <cstring>
#include <functional>
#include <bitset>
#include <mutex>

#include "SislObject.hpp"
#include "CallStrategies.hpp"

namespace sisl
{

  namespace priv
  {
  class Generic_Delegate
  {
    protected:
      priv::Generic_callStrategy* gs;

      std::bitset<3> flags;
      /*
        [0]:isDanglingSafe
        [1]:isFunctor
        [2]:isMember / !isStatic
     */

      std::weak_ptr<SislObject> wptr_checker;
      std::mutex mtx;

    public:

      ~Generic_Delegate();

      inline bool isDangling() const;
      inline bool isDanglingSafe() const;
      inline bool isFunctor() const;
      inline bool isMember() const;
      inline bool isStatic() const;
      inline bool sameFunction(const Generic_Delegate*) const;
      inline bool sameOwner(const Generic_Delegate*) const;

      template< typename RET , typename... ARGS >
      void call(void* sender,ARGS... args);

      inline bool operator<( const Generic_Delegate& target ) const;

      template< typename RET2 , typename... ARGS2 >
      inline bool operator==( const Generic_Delegate& target ) const;

      template< typename RET2 , typename... ARGS2 >
      inline bool operator!=( const Generic_Delegate& target ) const;

      inline void* getObject(){
        return gs->object;
      }

  };
}
}

#ifndef SISL_GENERIC_DELEGATE_TPP
#define SISL_GENERIC_DELEGATE_TPP
#include "../templates/Generic_Delegate.tpp"
#endif //SISL_GENERIC_DELEGATE_TPP

#endif //SISL_GENERIC_DELEGATE_HPP
