////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

#ifndef SISL_OBJECT_HPP
#define SISL_OBJECT_HPP

#include <memory>

namespace sisl
{
  class SislObject
  {
    public:
      std::shared_ptr<SislObject> __sisl__this;
      void* __sisl__sender = nullptr;
      SislObject(){
        __sisl__this = std::make_shared<SislObject>(*this);
      }

      template< typename T >
      inline T* getSender(){
        return reinterpret_cast<T*>(__sisl__sender);
      }
  };
}

#endif //SISL_OBJECT_HPP
