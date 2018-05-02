////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

#ifndef SISL_OBJECT_HPP
#define SISL_OBJECT_HPP

#include "Utils.h"

namespace sisl
{
  class SislObject : public enable_check_from_this<SislObject>
  {
    public:
      void* __sisl__sender = nullptr;

      SislObject() = default;

      SislObject(const SislObject&) = default;

      SislObject(SislObject&&) = default;

      inline bool isSenderKnown() const
      {
          return __sisl__sender != nullptr;
      }

      template< typename T >
      inline T getSender(){
          return reinterpret_cast<T>(__sisl__sender);
      }
  };
}

#endif //SISL_OBJECT_HPP
