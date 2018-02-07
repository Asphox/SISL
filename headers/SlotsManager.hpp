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

        uintptr_t connect_delegate(Generic_Delegate* gd);
        uintptr_t disconnect_delegate(Generic_Delegate* gd);

        void getDelegatesWithSameMemberFunction( const Generic_Delegate& gd , std::vector<uintptr_t>& ids );

      public:

        template< typename RET , typename... ARGS >
        uintptr_t onConnect( const std::function<RET(ARGS...)>& functor);

        template< typename OBJ , typename RET , typename... ARGS >
        uintptr_t onConnect(OBJ* obj, RET(OBJ::*fp)(ARGS...) );

        template< typename RET , typename... ARGS >
        uintptr_t onConnect( RET(*fp)(ARGS...) );

        template< typename OBJ , typename RET , typename... ARGS >
        uintptr_t onDisconnect(OBJ* obj , RET(OBJ::*fp)(ARGS...));

        template< typename RET , typename... ARGS >
        uintptr_t onDisconnect( RET(*fp)(ARGS...));

        void onDisconnect( uintptr_t id );

        void getDelegatesOwnedBy( void* obj , std::vector<uintptr_t>& related_ids );

        template< typename RET , typename OBJ , typename... ARGS >
        void getDelegatesWithSameMemberFunction( RET(OBJ::*fp)(ARGS...) , std::vector<uintptr_t>& related_ids );

        template< typename... ARGS >
        void call(void* sender , const uintptr_t id , ARGS... args);

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
