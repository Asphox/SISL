//
// Created by asphox on 02/05/18.
//

#ifndef SISL_UTILS_H
#define SISL_UTILS_H

#include <stdint-gcc.h>
#include <memory>

namespace sisl
{
    typedef uintptr_t Id;

    template< typename T >
    class check_ptr;

    template< typename T >
    class enable_check_from_this
    {
    private:
        friend class check_ptr<T>;
        std::shared_ptr<T*> addr;

    public:
        enable_check_from_this() : addr(std::make_shared<T*>(static_cast<T*>(this))){}
    };

    template< typename T >
    class check_ptr
    {
    private:
        std::weak_ptr<T*> target_to_check;

    public:
        check_ptr() = default;
        check_ptr(const check_ptr&) = default;
        check_ptr(check_ptr&&) = default;
        explicit check_ptr(enable_check_from_this<T>& target ) : target_to_check(target.addr) {}
        explicit check_ptr(enable_check_from_this<T>* target ) : target_to_check(target->addr){}
        inline check_ptr<T>& operator=(enable_check_from_this<T>& target){ target_to_check = target.addr;  }
        inline check_ptr<T>& operator=(enable_check_from_this<T>* target){ target_to_check = target->addr; }
        inline T* operator->() const { return get(); }
        T* get() const
        {
            if(valid())
                return target_to_check.lock().get();
            else
                return nullptr;
        }
        inline operator bool() const { return valid(); }
        bool valid() const { return !target_to_check.expired(); }


    };
}

#endif //SISL_UTILS_H
