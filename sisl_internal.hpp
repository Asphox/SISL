#ifndef SISL_INTERNAL_HPP
#define SISL_INTERNAL_HPP

#include <concepts>

#define __SISL_SIG_DEFINE(name, ...) SISL_NAMESPACE##::signal<__VA_ARGS__> name{*this};

#define __SISL_STR_DEFINE(x) #x
#define __SISL_STRINGIFY_DEFINE(x) __SISL_STR_DEFINE(x)

#define __SISL_BITWISE_ON_ENUM(enum_name) \
inline enum_name operator|(enum_name l, enum_name r) { return static_cast<enum_name>(static_cast<std::underlying_type_t<enum_name>>(l) | static_cast<std::underlying_type_t<enum_name>>(r)); } \
inline enum_name operator&(enum_name l, enum_name r) { return static_cast<enum_name>(static_cast<std::underlying_type_t<enum_name>>(l) & static_cast<std::underlying_type_t<enum_name>>(r)); } \
inline enum_name operator^(enum_name l, enum_name r) { return static_cast<enum_name>(static_cast<std::underlying_type_t<enum_name>>(l) ^ static_cast<std::underlying_type_t<enum_name>>(r)); } \
inline enum_name operator~(enum_name v) { return static_cast<enum_name>(~static_cast<std::underlying_type_t<enum_name>>(v)); } \
inline enum_name& operator|=(enum_name& l, enum_name& r) { l = l | r; return l; } \
inline enum_name& operator&=(enum_name& l, enum_name& r) { l = l & r; return l; } \
inline enum_name& operator^=(enum_name& l, enum_name& r) { l = l ^ r; return l; } \

namespace SISL_NAMESPACE
{
	namespace priv
	{
		class generic_object
		{
		public:
			[[nodiscard]] virtual std::weak_ptr<generic_object> get_weak_ptr() = 0;
		};

		template<typename T>
		concept SISL_OBJECT = std::is_base_of_v<generic_object, T>;

		thread_local generic_object* gtl_current_sender = nullptr;

		// traits for castable
		template<typename TFROM, typename TTO, typename = void>
		struct is_static_castable : std::false_type {};

		template<typename From, typename To>
		struct is_static_castable<From, To, std::void_t<decltype(static_cast<To>(std::declval<From>()))>> : std::true_type {};

		template<typename From, typename To>
		inline constexpr bool is_static_castable_v = is_static_castable<From, To>::value;

		// Helper to check static_castability of two tuples of types pairwise
		template<typename Tuple, typename TupleArgs>
		struct tuple_are_static_castable_impl;

		template<typename... TupleArgs, typename... TArgs>
		struct tuple_are_static_castable_impl<std::tuple<TupleArgs...>, std::tuple<TArgs...>> {
		private:
			// Check pairwise static_castability by index
			template<std::size_t... Is>
			static constexpr bool check(std::index_sequence<Is...>) {
				return (is_static_castable_v<std::tuple_element_t<Is, std::tuple<TArgs...>>,
					std::tuple_element_t<Is, std::tuple<TupleArgs...>>> && ...);
			}

		public:
			static constexpr bool value = sizeof...(TupleArgs) == sizeof...(TArgs) &&
				check(std::index_sequence_for<TupleArgs...>{});
		};

		// Main trait interface: 
		template<typename Tuple, typename... TArgs>
		struct tuple_is_static_castable : tuple_are_static_castable_impl<Tuple, std::tuple<TArgs...>> {};

		// Convenience variable template
		template<typename Tuple, typename... TArgs>
		inline constexpr bool tuple_is_static_castable_v = tuple_is_static_castable<Tuple, TArgs...>::value;

		// Traits for member functions
		template<typename T>
		struct member_function_traits;

		template<typename R, typename C, typename... ARGS>
		struct member_function_traits<R(C::*)(ARGS...)>
		{
			using return_type = R; using class_type = C; using argument_types = std::tuple<ARGS...>;
			static constexpr bool is_const = false;
			static constexpr bool is_volatile = false;
		};

		template<typename R, typename C, typename... ARGS>
		struct member_function_traits<R(C::*)(ARGS...) const>
		{
			using return_type = R; using class_type = C; using argument_types = std::tuple<ARGS...>;
			static constexpr bool is_const = true;
			static constexpr bool is_volatile = false;
		};

		template<typename R, typename C, typename... ARGS>
		struct member_function_traits<R(C::*)(ARGS...) volatile>
		{
			using return_type = R; using class_type = C; using argument_types = std::tuple<ARGS...>;
			static constexpr bool is_const = false;
			static constexpr bool is_volatile = true;
		};

		template<typename R, typename C, typename... ARGS>
		struct member_function_traits<R(C::*)(ARGS...) const volatile>
		{
			using return_type = R; using class_type = C; using argument_types = std::tuple<ARGS...>;
			static constexpr bool is_const = true;
			static constexpr bool is_volatile = true;
		};

		// Checks if TMETHOD is a member function pointer of class TCLASS
		// and if its parameter types exactly match the types TARGS...
		// This concept enforces an exact match of the argument types (no implicit conversions).
		template<typename TMETHOD, typename TCLASS, typename... TARGS>
		concept COMPATIBLE_METHOD_OF = requires
		{
			typename member_function_traits<TMETHOD>::return_type;
			typename member_function_traits<TMETHOD>::class_type;
			typename member_function_traits<TMETHOD>::argument_types;
		}
		&& std::is_same_v<typename member_function_traits<TMETHOD>::class_type, TCLASS>
		&& tuple_is_static_castable_v<typename member_function_traits<TMETHOD>::argument_types, TARGS...>;

		// Traits for functor
		// Add a helper for functors (deduce operator())
		template<typename T>
		struct functor_traits : member_function_traits<decltype(&T::operator())> {};

		template<typename T, typename = void>
		struct is_functor : std::false_type {};

		template<typename T>
		struct is_functor<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

		template<typename T>
		inline constexpr bool is_functor_v = is_functor<T>::value;

		template<typename Functor, typename... TArgs>
		concept COMPATIBLE_FUNCTOR = std::is_class_v<Functor> && requires
		{
			typename functor_traits<Functor>::return_type;
			typename functor_traits<Functor>::class_type;
			typename functor_traits<Functor>::argument_types;
		}
		&& tuple_is_static_castable_v<typename functor_traits<Functor>::argument_types, TArgs...>;

		// Traits for functions
		template<typename T>
		struct function_traits;

		template<typename R, typename... Args>
		struct function_traits<R(*)(Args...)>
		{
			using return_type = R;
			using argument_types = std::tuple<Args...>;
		};

		template<typename Function, typename... TArgs>
		concept COMPATIBLE_FUNCTION = requires {
			typename function_traits<Function>::return_type;
			typename function_traits<Function>::argument_types;
		}
		&& tuple_is_static_castable_v<typename function_traits<Function>::argument_types, TArgs...>;
	}
}

#endif //SISL_INTERNAL_HPP
