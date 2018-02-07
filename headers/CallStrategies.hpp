#ifndef SISL_CALLSTRATEGIES_HPP
#define SISL_CALLSTRATEGIES_HPP

#include <functional>
#include <cstring>

namespace sisl
{
  namespace priv
  {

    class _Impl_class;

    constexpr uint8_t size_biggest_fptr = sizeof(void(_Impl_class::*)());

    template< typename RET , typename... ARGS >
    using impl_mfptr = RET(_Impl_class::*)(ARGS...);

    template< typename RET , typename... ARGS >
    using impl_fptr = RET(*)(ARGS...);

    struct Generic_callStrategy
    {
      uint8_t raw_biggest_fptr[size_biggest_fptr];
      _Impl_class* object;
      Generic_callStrategy(){
        object=nullptr;
        memset(raw_biggest_fptr,0,size_biggest_fptr);
      }
    };

    template< typename RET , typename... ARGS >
    struct CallStrategy : public Generic_callStrategy
    {
      virtual RET call(ARGS...) = 0;
      virtual ~CallStrategy(){}
    };

    template< typename RET , typename... ARGS >
    struct Method_callStrategy : public CallStrategy<RET,ARGS...>
    {
      virtual RET call(ARGS... args){
        return (this->object->*(*reinterpret_cast<impl_mfptr<RET,ARGS...>*>(&this->raw_biggest_fptr)))(args...);
      }
    };

    template< typename RET , typename... ARGS >
    struct Static_callStrategy : public CallStrategy<RET,ARGS...>
    {
      virtual RET call(ARGS... args){
        return (*reinterpret_cast<impl_fptr<RET,ARGS...>*>(&this->raw_biggest_fptr))(args...);
      }
    };

    template< typename RET , typename... ARGS >
    struct Functor_callStrategy : public CallStrategy<RET,ARGS...>
    {
      virtual RET call(ARGS... args){
        return (*reinterpret_cast<std::function<RET(ARGS...)>*>(this->object))(args...);
      }
    };




  }
}

#endif //SISL_CALLSTRATEGIES_HPP
