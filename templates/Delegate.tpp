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

    template< typename RET , typename... ARGS >
    template< typename OBJ >
    void Delegate<RET,ARGS...>::init_with_member(OBJ* obj){
      flags[0] = std::is_base_of<Object,OBJ>::value && reinterpret_cast<uintptr_t>(obj) != 0;
      flags[1] = false;
      flags[2] = true;
      if(isDanglingSafe())
        checker = reinterpret_cast<Object*>(obj);
      gs->object = reinterpret_cast<priv::IMPLCLASS*>(obj);
    }

    template< typename RET , typename... ARGS >
    void Delegate<RET,ARGS...>::init_with_static(){
      flags[0] = false;
      flags[1] = false;
      flags[2] = false;
    }

    template< typename RET , typename... ARGS >
    Delegate<RET,ARGS...>::Delegate( const std::function<RET(ARGS...)>& functor )
    {
      gs = new priv::Functor_callStrategy<RET,ARGS...>;
      flags[0] = false;
      flags[1] = true;
      flags[2] = false;
      gs->object = reinterpret_cast<priv::IMPLCLASS*>(new std::function<RET(ARGS...)>);
      *reinterpret_cast<std::function<void(ARGS...)>*>(gs->object) = functor;
    }

    template< typename RET , typename... ARGS >
    template< typename OBJ >
    Delegate<RET,ARGS...>::Delegate( OBJ* obj , RET(OBJ::*fp)(ARGS...)){
      gs = new priv::Method_callStrategy<RET,ARGS...>;
      class IMPLCLASSTMP : public OBJ{};
      typedef RET(IMPLCLASSTMP::*generic_fp)(ARGS...);
      reinterpret_cast<generic_fp&>(gs->raw_biggest_fptr) = fp;
      init_with_member(obj);
    }

    template< typename RET , typename... ARGS >
    Delegate<RET,ARGS...>::Delegate( RET(*fp)(ARGS...) ){
      gs = new priv::Static_callStrategy<RET,ARGS...>;
      reinterpret_cast<priv::impl_fptr<ARGS...>&>(gs->raw_biggest_fptr) = fp;
      init_with_static();
    }

    template< typename RET , typename... ARGS >
    template< typename... ARGS2 >
    inline void Delegate<RET,ARGS...>::call(ARGS2... args){
      operator()(args...);
    }

    template< typename RET , typename... ARGS >
    template< typename... ARGS2 >
    void Delegate<RET,ARGS...>::operator()(ARGS2... args){
      static_assert( sizeof...(ARGS2) != sizeof...(ARGS)-1," [SISL] Delegate called with bad number of arguments ! ");
      static_cast<priv::CallStrategy<ARGS...>*>(gs)->call(args...);
    }
}
}
