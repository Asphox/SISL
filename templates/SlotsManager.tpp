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
    uintptr_t SlotsManager::connect_delegate(Generic_Delegate* gd){
      auto checker = slots.insert({gd,1});
      if(!checker.second)
      {
        delete gd;
        ++checker.first->second;
      }
      return reinterpret_cast<uintptr_t>(checker.first->first);
    }

    uintptr_t SlotsManager::disconnect_delegate(Generic_Delegate* gd){
      uintptr_t id_ret = 0;
      auto it = slots.find(gd);
      delete gd;
      if( it != slots.end() )
      {
        id_ret = reinterpret_cast<uintptr_t>(it->first);
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
    uintptr_t SlotsManager::onConnect( const std::function<RET(ARGS...)>& functor){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(functor);
      return connect_delegate(gd);
    }

    template< typename OBJ , typename RET , typename... ARGS >
    uintptr_t SlotsManager::onConnect(OBJ* obj, RET(OBJ::*fp)(ARGS...)){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(obj,fp);
      return connect_delegate(gd);
    }

    template< typename RET , typename... ARGS >
    uintptr_t SlotsManager::onConnect( RET(*fp)(ARGS...)){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(fp);
      return connect_delegate(gd);
    }

    template< typename OBJ , typename RET , typename... ARGS >
    uintptr_t SlotsManager::onDisconnect(OBJ* obj , RET(OBJ::*fp)(ARGS...) ){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(obj,fp);
      return disconnect_delegate(gd);

    }

    template< typename RET , typename... ARGS >
    uintptr_t SlotsManager::onDisconnect(RET(*fp)(ARGS...) ){
      Generic_Delegate* gd = new Delegate<RET,ARGS...>(fp);
      return disconnect_delegate(gd);
    }

    void SlotsManager::onDisconnect( uintptr_t id ){
      auto it = slots.find(reinterpret_cast<Generic_Delegate*>(id));
      if( it->second == 1 )
      {
        delete it->first;
        slots.erase(it);
      }
      else
      {
        --it->second;
      }
    }

    void SlotsManager::getDelegatesOwnedBy( void* obj , std::vector<uintptr_t>& related_ids ){
      for( auto& it : slots )
      {
        if( it.first->getObject() == obj )
          related_ids.push_back(reinterpret_cast<uintptr_t>(it.first));
      }
    }

    template< typename RET, typename OBJ , typename... ARGS >
    void SlotsManager::getDelegatesWithSameMemberFunction( RET(OBJ::*fp)(ARGS...) , std::vector<uintptr_t>& related_ids ){
      const Generic_Delegate& gd = Delegate<RET,ARGS...>(reinterpret_cast<OBJ*>(0),fp);
      for( auto& it : slots )
      {
        if( it.first->sameFunction(&gd) )
          related_ids.push_back(reinterpret_cast<uintptr_t>(it.first));
      }
    }

    template< typename... ARGS >
    void SlotsManager::call(void* sender , const uintptr_t id , ARGS... args){
      auto it = slots.find(reinterpret_cast<Generic_Delegate*>(id));
      if(sender && it->first->isDanglingSafe())
        reinterpret_cast<SislObject*>(it->first->getObject())->__sisl__sender = sender;
      if( it != slots.end() )
        it->first->call<void,ARGS...>(args...);
      if(sender && it->first->isDanglingSafe())
        reinterpret_cast<SislObject*>(it->first->getObject())->__sisl__sender = nullptr;
    }

    SlotsManager::~SlotsManager(){
      for( auto it : slots )
        delete it.first;
      slots.clear();
    }
  }
}
