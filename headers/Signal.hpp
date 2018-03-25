////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

#ifndef SISL_SIGNAL_HPP
#define SISL_SIGNAL_HPP

#include <vector>
#include <algorithm>
#include <functional>
#include <cstddef>
#include "SlotsManager.hpp"

namespace sisl
{
  template< typename... ARGS >
  class Signal
  {
    private:
      std::vector<uintptr_t> slots_index;
      void* owner;

      void connect( uintptr_t id );
      void disconnect( uintptr_t id );
      void disconnect_all( std::vector<uintptr_t>& ids);

    public:

      Signal(const Signal&) = default;

      Signal(Signal&&) = default;

      Signal( void* Owner = nullptr ) : owner(Owner)
      {}

      /*==============================================
      Connect a functor (std::function or lambda)
      ===============================================*/
      void connect(const std::function<void(ARGS...)>& std_fct);

      /*==============================================
      Connect a member function
      ===============================================*/
      template< typename OBJ , typename RET >
      void connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) );

      /*==============================================
      Connect a const member function
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const);

      /*==============================================
      Connect a volatile member function
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) volatile);

      /*==============================================
      Connect a const volatile member function
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const volatile);

      /*==============================================
      Connect a static function
      ===============================================*/
      template< typename RET >
      void connect( RET(*fp)(ARGS...) );

      /*==============================================
      Disconnect a member function
      ===============================================*/
      template< typename OBJ , typename RET >
      void disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) );

      /*==============================================
      Disconnect a const member function
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const);

      /*==============================================
      Disconnect a volatile member function
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) volatile);

      /*==============================================
      Disconnect a const volatile member function
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const volatile);

      /*==============================================
      Disconnect a static function
      ===============================================*/
      template< typename RET >
      void disconnect( RET(*fp)(ARGS...) );

      /*==============================================
      Disconnect all functions and functors
      ===============================================*/
      void disconnect_all();

      /*==============================================
      Disconnect all member functions owned by obj
      ===============================================*/
      void disconnect_all( void* obj );

      /*==============================================
      Disconnect fp member functions owned by all OBJ instances
      ===============================================*/
      template< typename OBJ , typename RET >
      void disconnect_all( RET(OBJ::*fp)(ARGS...) );

      /*==============================================
      Disconnect fp const member functions owned by all OBJ instances
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void disconnect_all( RET(OBJ::*fp)(ARGS...) const );

      /*==============================================
      Disconnect fp volatile member functions owned by all OBJ instances
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void disconnect_all( RET(OBJ::*fp)(ARGS...) volatile );

      /*==============================================
      Disconnect fp const volatile member functions owned by all OBJ instances
      ===============================================*/
      template< typename OBJ , typename RET >
      inline void disconnect_all( RET(OBJ::*fp)(ARGS...) const volatile );

      /*==============================================
      Emission of the signal
      ===============================================*/
      template< typename... ARGS2 >
      inline void operator()(ARGS2... args){ emit(args...); }

      /*==============================================
      Emission of the signal
      ===============================================*/
      template< typename... ARGS2 >
      void emit(ARGS2... args);

  };
}

#ifndef SISL_SIGNAL_TPP
#define SISL_SIGNAL_TPP
#include "../templates/Signal.tpp"
#endif //SISL_SIGNAL_TPP

#endif //SISL_SIGNAL_HPP
