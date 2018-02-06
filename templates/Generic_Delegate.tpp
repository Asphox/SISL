////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////


#include "../headers/Delegate.hpp"

namespace sisl
{

  Generic_Delegate::~Generic_Delegate(){
    if(isFunctor()) delete reinterpret_cast<std::function<void()>*>(id.object);
  }

  template< typename RET , typename... ARGS >
  RET Generic_Delegate::call(ARGS... args){
    return static_cast<Delegate<RET,ARGS...>*>(this)->call(args...);
  }

  inline bool Generic_Delegate::isDangling() const{
    return isDanglingSafe() && wptr_checker.expired();
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
    return (isMember() && target->isMember() && id.object == target->id.object);
  }

  inline bool Generic_Delegate::sameFunction( const Generic_Delegate* target )const{
    return (!isFunctor() && isMember() == target->isMember() && memcmp(id.raw_biggest_fptr,target->id.raw_biggest_fptr,sizeof(id.raw_biggest_fptr)) == 0);
  }

  inline bool Generic_Delegate::operator<( const Generic_Delegate& target ) const{
    return memcmp(&id,&target.id,sizeof(id)) < 0;
  }

  template< typename RET2 , typename... ARGS2 >
  inline bool Generic_Delegate::operator==( const Generic_Delegate& target ) const{
    return memcmp(&id,&target.id,sizeof(id)) == 0;
  }

  template< typename RET2 , typename... ARGS2 >
  inline bool Generic_Delegate::operator!=( const Generic_Delegate& target ) const{
    return memcmp(&id,&target.id,sizeof(id)) != 0;
  }
}
