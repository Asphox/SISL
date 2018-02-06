////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

namespace sisl
{
    template< typename RET , typename... ARGS >
    template< typename OBJ >
    void Delegate<RET,ARGS...>::init_with_member(OBJ* obj, RET(OBJ::*fp)(ARGS...)){
      flags[0] = std::is_base_of<SislObject,OBJ>::value && reinterpret_cast<uintptr_t>(obj) != 0;
      flags[1] = false;
      flags[2] = true;
      if(isDanglingSafe())
        wptr_checker = reinterpret_cast<SislObject*>(obj)->__sisl__this;
      memset(&id,0,sizeof(id));
      id.object = reinterpret_cast<_Impl_class*>(obj);
      class _Impl2_class : public OBJ{};
      typedef RET(_Impl2_class::*generic_fp)(ARGS...);
      reinterpret_cast<generic_fp&>(id.raw_biggest_fptr) = fp;
    }

    template< typename RET , typename... ARGS >
    Delegate<RET,ARGS...>::Delegate( const std::function<RET(ARGS...)>& functor ){
      flags[0] = false;
      flags[1] = true;
      flags[2] = false;
      id.object = reinterpret_cast<_Impl_class*>(new std::function<RET(ARGS...)>);
      *reinterpret_cast<std::function<RET(ARGS...)>*>(id.object) = functor;
    }

    template< typename RET , typename... ARGS >
    template< typename OBJ >
    Delegate<RET,ARGS...>::Delegate( OBJ* obj , RET(OBJ::*fp)(ARGS...)){
      init_with_member(obj,fp);
    }

    template< typename RET , typename... ARGS >
    template< typename OBJ >
    Delegate<RET,ARGS...>::Delegate( OBJ* obj , RET(OBJ::*fp)(ARGS...) const ){
      init_with_member(obj,const_cast<RET(OBJ::*)(ARGS...)>(fp));
    }

    template< typename RET , typename... ARGS >
    template< typename OBJ >
    Delegate<RET,ARGS...>::Delegate( OBJ* obj , RET(OBJ::*fp)(ARGS...) volatile ){
      init_with_member(obj,const_cast<RET(OBJ::*)(ARGS...)>(fp));
    }

    template< typename RET , typename... ARGS >
    template< typename OBJ >
    Delegate<RET,ARGS...>::Delegate( OBJ* obj , RET(OBJ::*fp)(ARGS...) const volatile ){
      init_with_member(obj,const_cast<RET(OBJ::*)(ARGS...)>(fp));
    }

    template< typename RET , typename... ARGS >
    Delegate<RET,ARGS...>::Delegate( RET(*fp)(ARGS...) ){
      flags[0] = false;
      flags[1] = false;
      flags[2] = false;
      memset(id.raw_biggest_fptr,0,sizeof(mfptr<RET,ARGS...>));
      id.object = nullptr;
      reinterpret_cast<fptr<RET,ARGS...>&>(id.raw_biggest_fptr) = fp;
    }

    template< typename RET , typename... ARGS >
    template< typename... ARGS2 >
    inline RET Delegate<RET,ARGS...>::call(ARGS2... args){
      return operator()(args...);
    }

    template< typename RET , typename... ARGS >
    template< typename... ARGS2 >
    RET Delegate<RET,ARGS...>::operator()(ARGS2... args){
      static_assert( sizeof...(ARGS2) != sizeof...(ARGS)-1," [SISL] Delegate called with bad number of arguments ! ");
      if(isFunctor())
                    return (*reinterpret_cast<std::function<RET(ARGS2...)>*>(id.object))(args...);
      if(id.object) return (id.object->*(*reinterpret_cast<mfptr<RET,ARGS...>*>(&id.raw_biggest_fptr)))(args...);
      else          return (*(*reinterpret_cast<fptr<RET,ARGS...>*>(&id.raw_biggest_fptr)))(args...);
    }


  template< typename RET , typename OBJ , typename... ARGS >
  inline Delegate<RET,ARGS...> make_delegate(OBJ* obj, RET(OBJ::*fp)(ARGS...) ){
    return Delegate<RET,ARGS...>(obj,fp);
  }

  template< typename RET , typename OBJ , typename... ARGS >
  inline Delegate<RET,ARGS...> make_delegate(OBJ* obj, RET(OBJ::*fp)(ARGS...) const ){
    return Delegate<RET,ARGS...>(obj,fp);
  }

  template< typename RET , typename OBJ , typename... ARGS >
  inline Delegate<RET,ARGS...> make_delegate(OBJ* obj, RET(OBJ::*fp)(ARGS...) volatile ){
    return Delegate<RET,ARGS...>(obj,fp);
  }

  template< typename RET , typename OBJ , typename... ARGS >
  inline Delegate<RET,ARGS...> make_delegate(OBJ* obj, RET(OBJ::*fp)(ARGS...) const volatile ){
    return Delegate<RET,ARGS...>(obj,fp);
  }

  template< typename RET , typename... ARGS >
  inline Delegate<RET,ARGS...> make_delegate(RET(*fp)(ARGS...)){
    return Delegate<RET,ARGS...>(fp);
  }
}
