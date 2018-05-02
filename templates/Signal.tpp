////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

namespace sisl
{
  template< typename... ARGS >
  Id Signal<ARGS...>::connect_id( Id id ){
    auto it = std::find(slots_index.begin(),slots_index.end(),id);
    if(it == slots_index.end())
      slots_index.push_back(id);
    return id;
  }

  template< typename... ARGS >
  void Signal<ARGS...>::disconnect_id( Id id ){
    auto it = std::find(slots_index.begin(),slots_index.end(),id);
    if( it != slots_index.end() )
      slots_index.erase(it);
  }

  template< typename... ARGS >
  void Signal<ARGS...>::disconnect_all_id( std::vector<Id>& ids ){
    auto it = slots_index.begin();
    for( size_t i=0 ; i<ids.size(); i++ )
    {
      it = std::find(slots_index.begin(),slots_index.end(),ids[i]);
      if( it != slots_index.end() )
      {
        priv::slotsManager.onDisconnect(ids[i]);
        slots_index.erase(it);
      }
    }
  }

  template< typename... ARGS >
  Id Signal<ARGS...>::connect( const std::function<void(ARGS...)>& functor){
    return connect_id(priv::slotsManager.onConnect(functor));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  Id Signal<ARGS...>::connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) ){
    return connect_id(priv::slotsManager.onConnect(obj,fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline Id Signal<ARGS...>::connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const){
    return connect_id(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline Id Signal<ARGS...>::connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) volatile){
    return connect_id(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline Id Signal<ARGS...>::connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const volatile){
    return connect_id(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename RET >
  Id Signal<ARGS...>::connect( RET(*fp)(ARGS...) ){
    return connect_id(priv::slotsManager.onConnect(fp));
  }

  template< typename... ARGS >
  inline void Signal<ARGS...>::disconnect(Id id){
    disconnect_id(priv::slotsManager.onDisconnect(id));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...)){
    disconnect_id(priv::slotsManager.onDisconnect(obj,fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const ){
    disconnect(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) volatile ){
    disconnect(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const volatile ){
    disconnect(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename RET >
  inline void Signal<ARGS...>::disconnect( RET(*fp)(ARGS...)){
    disconnect_id(priv::slotsManager.onDisconnect(fp));
  }

  template< typename... ARGS >
  void Signal<ARGS...>::disconnect_all(){
    for( Id& it : slots_index )
    {
      priv::slotsManager.onDisconnect(it);
    }
    slots_index.clear();
  }

  template< typename... ARGS >
  void Signal<ARGS...>::disconnect_all( void* obj ){
    std::vector<Id> ids;
    priv::slotsManager.getDelegatesOwnedBy(obj,ids);
    disconnect_all(ids);
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  void Signal<ARGS...>::disconnect_all( RET(OBJ::*fp)(ARGS...) ){
    std::vector<Id> ids;
    priv::slotsManager.getDelegatesWithSameMemberFunction(fp,ids);
    disconnect_all(ids);
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::disconnect_all( RET(OBJ::*fp)(ARGS...) const ){
    disconnect_all(reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::disconnect_all( RET(OBJ::*fp)(ARGS...) volatile ){
    disconnect_all(reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::disconnect_all( RET(OBJ::*fp)(ARGS...) const volatile ){
    disconnect_all(reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename... ARGS2 >
  void Signal<ARGS...>::emit(ARGS2... args){
    for( auto it = slots_index.begin() ; it != slots_index.end(); )
    {
        if(!priv::slotsManager.call<ARGS...>(owner,*it,static_cast<ARGS>(args)...))
            it = slots_index.erase(it);
        else
            ++it;
    }
  }
}
