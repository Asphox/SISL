////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

namespace sisl
{
  template< typename... ARGS >
  void Signal<ARGS...>::connect( uintptr_t id ){
    auto it = std::find(slots_index.begin(),slots_index.end(),id);
    if(it == slots_index.end())
      slots_index.push_back(id);
  }

  template< typename... ARGS >
  void Signal<ARGS...>::disconnect( uintptr_t id ){
    auto it = std::find(slots_index.begin(),slots_index.end(),id);
    if( it != slots_index.end() )
      slots_index.erase(it);
  }

  template< typename... ARGS >
  void Signal<ARGS...>::disconnect_all( std::vector<uintptr_t>& ids ){
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
  void Signal<ARGS...>::connect( const std::function<void(ARGS...)>& functor){
    slots_index.push_back(priv::slotsManager.onConnect(functor));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  void Signal<ARGS...>::connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) ){
    connect(priv::slotsManager.onConnect(obj,fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const){
    connect(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) volatile){
    connect(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::connect( OBJ* obj , RET(OBJ::*fp)(ARGS...) const volatile){
    connect(obj,reinterpret_cast<RET(OBJ::*)(ARGS...)>(fp));
  }

  template< typename... ARGS >
  template< typename RET >
  void Signal<ARGS...>::connect( RET(*fp)(ARGS...) ){
    connect(priv::slotsManager.onConnect(fp));
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  inline void Signal<ARGS...>::disconnect( OBJ* obj , RET(OBJ::*fp)(ARGS...)){
    disconnect(priv::slotsManager.onDisconnect(obj,fp));
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
    disconnect(priv::slotsManager.onDisconnect(fp));
  }

  template< typename... ARGS >
  void Signal<ARGS...>::disconnect_all(){
    for( uintptr_t& it : slots_index )
    {
      priv::slotsManager.onDisconnect(it);
    }
    slots_index.clear();
  }

  template< typename... ARGS >
  void Signal<ARGS...>::disconnect_all( void* obj ){
    std::vector<uintptr_t> ids;
    priv::slotsManager.getDelegatesOwnedBy(obj,ids);
    disconnect_all(ids);
  }

  template< typename... ARGS >
  template< typename OBJ , typename RET >
  void Signal<ARGS...>::disconnect_all( RET(OBJ::*fp)(ARGS...) ){
    std::vector<uintptr_t> ids;
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
