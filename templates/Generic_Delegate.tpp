////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////


#include "../headers/Delegate.hpp"

namespace sisl
{
  namespace priv
  {

  Generic_Delegate::~Generic_Delegate(){
    if(isFunctor()) delete reinterpret_cast<std::function<void()>*>(gs->object);
    delete static_cast<priv::CallStrategy<>*>(gs);
  }

  template< typename RET , typename... ARGS >
  void Generic_Delegate::call(void* sender,ARGS... args){
    std::lock_guard<std::mutex> lck(mtx);
    if( isDanglingSafe() && !isDangling() ) {
        reinterpret_cast<Object *>(getObject())->__sisl__sender = sender;
        static_cast<Delegate<RET, ARGS...> *>(this)->call(args...);
        reinterpret_cast<Object *>(getObject())->__sisl__sender = nullptr;
    }
    else{
        static_cast<Delegate<RET, ARGS...> *>(this)->call(args...);
    }
  }

  inline bool Generic_Delegate::isDangling() const{
    return isDanglingSafe() && !checker.valid();
  }

  inline bool Generic_Delegate::isDanglingSafe() const{
    return flags[0];
  }

  inline bool Generic_Delegate::isFunctor() const{
    return flags[1];
  }

  inline bool Generic_Delegate::isMember() const{
    return flags[2];
  }

  inline bool Generic_Delegate::isStatic() const{
    return !flags[2];
  }

  inline bool Generic_Delegate::sameOwner(const Generic_Delegate* target) const{
    return (isMember() && target->isMember() && gs->object == target->gs->object);
  }

  inline bool Generic_Delegate::sameFunction( const Generic_Delegate* target )const{
    return (!isFunctor() && isMember() == target->isMember() && memcmp(gs->raw_biggest_fptr,target->gs->raw_biggest_fptr,sizeof(priv::SIZE_BIGGEST_FPTR)) == 0);
  }

  inline bool Generic_Delegate::operator<( const Generic_Delegate& target ) const{
    return memcmp(gs,target.gs,sizeof(*gs)) < 0;
  }

  template< typename RET2 , typename... ARGS2 >
  inline bool Generic_Delegate::operator==( const Generic_Delegate& target ) const{
    return memcmp(gs,target.gs,sizeof(*gs)) == 0;
  }

  template< typename RET2 , typename... ARGS2 >
  inline bool Generic_Delegate::operator!=( const Generic_Delegate& target ) const{
    return memcmp(gs,target.gs,sizeof(*gs)) != 0;
  }
}
}
