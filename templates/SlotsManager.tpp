////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////
namespace sisl
{
  namespace priv
  {

    void SlotsManager::getDelegatesWithSameMemberFunction( const Generic_Delegate& gd , std::vector<Id>& ids){
      for( auto& it : slots )
      {
        if( it.first->sameFunction(&gd) )
          ids.push_back(reinterpret_cast<Id>(it.first));
      }
    }

    Id SlotsManager::connect_delegate(Generic_Delegate* gd){
      auto checker = slots.insert({gd,1});
      if(!checker.second)
      {
        delete gd;
        ++checker.first->second;
      }
      return reinterpret_cast<Id>(checker.first->first);
    }

    Id SlotsManager::disconnect_delegate(Generic_Delegate* gd){
      Id id_ret = 0;
      auto it = slots.find(gd);
      delete gd;
      if( it != slots.end() )
      {
        id_ret = reinterpret_cast<Id>(it->first);
        if( it->second == 1 ) //test & 1
        {
          delete it->first;
          slots.erase(it);
        }
        else
        {
          --it->second;
        }
      }
      return id_ret;
    }

    template< typename RET , typename... ARGS >
    Id SlotsManager::onConnect( const std::function<RET(ARGS...)>& functor){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(functor);
      return connect_delegate(gd);
    }

    template< typename OBJ , typename RET , typename... ARGS >
    Id SlotsManager::onConnect(OBJ* obj, RET(OBJ::*fp)(ARGS...) ){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(obj,fp);
      return connect_delegate(gd);
    }

    template< typename RET , typename... ARGS >
    Id SlotsManager::onConnect( RET(*fp)(ARGS...)){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(fp);
      return connect_delegate(gd);
    }

    template< typename OBJ , typename RET , typename... ARGS >
    Id SlotsManager::onDisconnect(OBJ* obj , RET(OBJ::*fp)(ARGS...) ){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(obj,fp);
      return disconnect_delegate(gd);
    }

    template< typename RET , typename... ARGS >
    Id SlotsManager::onDisconnect(RET(*fp)(ARGS...) ){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(fp);
      return disconnect_delegate(gd);
    }

    Id SlotsManager::onDisconnect( Id id ){
      auto it = slots.find(reinterpret_cast<Generic_Delegate*>(id));
      if( it != slots.end() )
      {
          if( it->second == 1 )
          {
              delete it->first;
              slots.erase(it);
          }
          else
          {
              --it->second;
          }
          return id;
      }
      else
      {
        return 0;
      }
    }

    void SlotsManager::getDelegatesOwnedBy( void* obj , std::vector<Id>& related_ids ){
      for( auto& it : slots )
      {
        if( it.first->getObject() == obj )
          related_ids.push_back(reinterpret_cast<Id>(it.first));
      }
    }

    template< typename RET, typename OBJ , typename... ARGS >
    void SlotsManager::getDelegatesWithSameMemberFunction( RET(OBJ::*fp)(ARGS...) , std::vector<Id>& ids ){
      getDelegatesWithSameMemberFunction(Delegate<RET,ARGS...>(reinterpret_cast<OBJ*>(0),fp));
    }

    template< typename... ARGS >
    bool SlotsManager::call(void* sender , const Id id , ARGS... args){
      auto it = slots.find(reinterpret_cast<Generic_Delegate*>(id));
      if( it == slots.end() )
        return false;
      if( it->first->isDanglingSafe() && it->first->isDangling() )
      {
        slots.erase(it);
        return false;
      }
      else
      {
        it->first->call<void,ARGS...>(sender,args...);
        return true;
      }
    }

    SlotsManager::~SlotsManager(){
      for( auto it : slots )
        delete it.first;
      slots.clear();
    }


  }
}
