#ifndef SISL_CALLSTRATEGIES_HPP
#define SISL_CALLSTRATEGIES_HPP

#include <functional>
#include <cstring>

namespace sisl
{
  namespace priv
  {

    class IMPLCLASS;

    constexpr uint8_t SIZE_BIGGEST_FPTR = sizeof(void(IMPLCLASS::*)());

    template< typename... ARGS >
    using impl_mfptr = void(IMPLCLASS::*)(ARGS...);

    template< typename... ARGS >
    using impl_fptr = void(*)(ARGS...);

    struct Generic_callStrategy
    {
      uint8_t raw_biggest_fptr[SIZE_BIGGEST_FPTR];
      IMPLCLASS* object;
      
      Generic_callStrategy(){
        object=nullptr;
        memset(raw_biggest_fptr,0,SIZE_BIGGEST_FPTR);
      }
    };

    template< typename... ARGS >
    struct CallStrategy : public Generic_callStrategy
    {
      virtual void call(ARGS...) = 0;
      virtual ~CallStrategy() = default;
    };

    template< typename RET , typename... ARGS >
    struct Method_callStrategy : public CallStrategy<ARGS...>
    {
      virtual void call(ARGS... args){
        (this->object->*(*reinterpret_cast<impl_mfptr<ARGS...>*>(&this->raw_biggest_fptr)))(args...);
      }
    };

    template< typename RET , typename... ARGS >
    struct Static_callStrategy : public CallStrategy<ARGS...>
    {
      virtual void call(ARGS... args){
        (*reinterpret_cast<impl_fptr<ARGS...>*>(&this->raw_biggest_fptr))(args...);
      }
    };

    template< typename RET , typename... ARGS >
    struct Functor_callStrategy : public CallStrategy<ARGS...>
    {
      virtual void call(ARGS... args){
        (*reinterpret_cast<std::function<void(ARGS...)>*>(this->object))(args...);
      }
    };




  }
}

#endif //SISL_CALLSTRATEGIES_HPP
