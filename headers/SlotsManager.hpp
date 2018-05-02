////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

#ifndef SISL_SLOTSMANAGER_HPP
#define SISL_SLOTSMANAGER_HPP

#include <memory>
#include <map>
#include <vector>
#include "Generic_Delegate.hpp"
#include "Delegate.hpp"
#include "Utils.h"

namespace sisl
{
  namespace priv
  {
    struct ptr_generic_delegate_cmp
    {
      bool operator()( const Generic_Delegate* a , const Generic_Delegate* b) const {
        return *a<*b;
      }
    };

    class SlotsManager
    {
      private:
        std::map<Generic_Delegate*,uint16_t,ptr_generic_delegate_cmp> slots;

        Id connect_delegate(Generic_Delegate* gd);
        Id disconnect_delegate(Generic_Delegate* gd);

        void getDelegatesWithSameMemberFunction( const Generic_Delegate& gd , std::vector<Id>& ids );

      public:

        template< typename RET , typename... ARGS >
        Id onConnect( const std::function<RET(ARGS...)>& functor);

        template< typename OBJ , typename RET , typename... ARGS >
        Id onConnect(OBJ* obj, RET(OBJ::*fp)(ARGS...) );

        template< typename RET , typename... ARGS >
        Id onConnect( RET(*fp)(ARGS...) );

        template< typename OBJ , typename RET , typename... ARGS >
        Id onDisconnect(OBJ* obj , RET(OBJ::*fp)(ARGS...));

        template< typename RET , typename... ARGS >
        Id onDisconnect( RET(*fp)(ARGS...));

        Id onDisconnect( Id id );

        void getDelegatesOwnedBy( void* obj , std::vector<Id>& related_ids );

        template< typename RET , typename OBJ , typename... ARGS >
        void getDelegatesWithSameMemberFunction( RET(OBJ::*fp)(ARGS...) , std::vector<Id>& related_ids );

        template< typename... ARGS >
        bool call(void* sender , const Id id , ARGS... args);

        ~SlotsManager();

    };
    SlotsManager slotsManager;
  }

}

#ifndef SISL_SLOTSMANAGER_TPP
#define SISL_SLOTSMANAGER_TPP
#include "../templates/SlotsManager.tpp"
#endif //SISL_SLOTSMANAGER_TPP

#endif //SISL_SLOTSMANAGER_HPP
