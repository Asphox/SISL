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
#include "SlotsManager.hpp"

namespace sisl
{
  template< typename... ARGS >
  class Signal
  {
    private:
      std::vector<uintptr_t> slots_index;
      void* owner;
    public:
      Signal(void* Owner = nullptr) : owner(Owner)
      {}

      void connect(const std::function<void(ARGS...)>& std_fct);

      template< typename OBJ , typename RET >
      void connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) );

      template< typename OBJ , typename RET >
      inline void connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const);

      template< typename OBJ , typename RET >
      inline void connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) volatile);

      template< typename OBJ , typename RET >
      inline void connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const volatile);

      template< typename RET >
      void connect( RET(*fp)(ARGS...) );

      template< typename OBJ , typename RET >
      void disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) );

      template< typename OBJ , typename RET >
      inline void disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const);

      template< typename OBJ , typename RET >
      inline void disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) volatile);

      template< typename OBJ , typename RET >
      inline void disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const volatile);

      template< typename RET >
      void disconnect( RET(*fp)(ARGS...) );

      void disconnect_all();

      void disconnect_all( void* obj );

      template< typename OBJ , typename RET >
      void disconnect_all( RET(OBJ::*fp)(ARGS...) );

      template< typename OBJ , typename RET >
      inline void disconnect_all( RET(OBJ::*fp)(ARGS...) const );

      template< typename OBJ , typename RET >
      inline void disconnect_all( RET(OBJ::*fp)(ARGS...) volatile );

      template< typename OBJ , typename RET >
      inline void disconnect_all( RET(OBJ::*fp)(ARGS...) const volatile );

      template< typename... ARGS2 >
      void operator()(ARGS2... args);
  };
}

#ifndef SISL_SIGNAL_TPP
#define SISL_SIGNAL_TPP
#include "../templates/Signal.tpp"
#endif //SISL_SIGNAL_TPP

#endif //SISL_SIGNAL_HPP
