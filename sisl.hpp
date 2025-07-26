#ifndef SISL_HPP
#define SISL_HPP

// <=====================================================================================>
// <=============					Configuration							=============>
// <=====================================================================================>


#ifndef SISL_NAMESPACE
/**
 * @def SISL_NAMESPACE
 * @brief Specifies the namespace under which all SISL symbols are defined.
 *
 * This macro defines the namespace used to encapsulate all SISL-related types and functions.
 * It can be redefined prior to including SISL headers if integration into an existing
 * project-specific namespace hierarchy is required.
 */
#define SISL_NAMESPACE sisl
#endif //SISL_NAMESPACE

#ifndef emit
 /**
  * @def emit
  * @brief Semantic macro used to denote signal emission.
  *
  * This macro serves purely as a syntactic aid to clearly indicate where a signal
  * is emitted within the code. It has no functional impact and exists solely to improve
  * code readability and intent expression.
  * 
  * @code
  * emit on_click(); // Emitting the signal
  * @endcode
  */
#define emit
#endif //emit

// <=====================================================================================>
// <=====================================================================================>
// <=====================================================================================>

#define sisl_sig __SISL_SIG_DEFINE

#include <optional>
#include <vector>
#include <functional>
#include <thread>
#include <memory>
#include "sisl_internal.hpp"


namespace SISL_NAMESPACE
{
	/**
	* @brief Retrieves the sender of the currently executing signal.
	*
	* This utility function returns a pointer to the object that emitted the currently
	* executing signal, casted to the appropriate type.
	*
	* @tparam TSENDER The expected type of the sender.
	* @return pointer to the sender object. if nullptr, the current execution is not in a signal context.
	*
	* @throws std::bad_cast If the current sender is not of type TSENDER.
	*/
	template<typename TSENDER>
	inline TSENDER* sender() { return priv::gtl_current_sender ? &dynamic_cast<TSENDER&>(*priv::gtl_current_sender) : nullptr; }


	/**
	 * @brief Base class for all signal-owning objects.
	 *
	 * Derive your types from `sisl::object<T>` to enable them to own and emit signals.
	 * This class integrates signal ownership semantics and weak pointer retrieval.
	 *
	 * @tparam TCRTP The derived class (CRTP pattern).
	 */
	template<typename TCRTP>
	class object : public std::enable_shared_from_this<TCRTP>, public priv::generic_object
	{
	public:

	protected:
		using priv::generic_object::get_weak_ptr;
		[[nodiscard]] std::weak_ptr<generic_object> get_weak_ptr() override 
		{ 
			std::weak_ptr<priv::generic_object> weak_generic;
			auto weak_derived = weak_from_this();
			if (auto shared_derived = weak_derived.lock())
			{
				weak_generic = std::static_pointer_cast<priv::generic_object>(shared_derived);
			}
			return weak_generic;
		}
	};

	/**
	* @enum type_connection
	* @brief Enumerates available connection policies for slots.
	*
	* These policies define how signals dispatch connected slots.
	²*/
	enum class type_connection
	{
		automatic		= 0,			///< Automatically choose between direct and queued.
		direct			= 1,			///< Call slot immediately in the emitter's thread.
		queued			= 2,			///< Enqueue slot to be invoked in the receiver's thread.
		blocking_queued	= 3,			///< Enqueue and block until the slot has finished.
		unique			= 1<<16,		///< Prevent multiple connections to the same slot.
		single_shot		= 1<<17,		///< Automatically disconnect after first trigger.
	};
	__SISL_BITWISE_ON_ENUM(type_connection)

	template<typename... TARGS>
	struct delegate
	{
		delegate(std::function<void(TARGS...)>&& callee, intptr_t object_id, std::size_t function_id)
		{
			m_callee = std::move(callee);
			m_id.object = object_id;
			m_id.function = function_id;
		}
		std::function<void(TARGS...)> m_callee;
		struct
		{
			intptr_t		object;
			std::size_t		function;
		}m_id;
		type_connection m_type = type_connection::automatic;
	};
	
	/**
	* @class signal
	* @brief Represents a signal that can notify connected slots.
	*
	* The `signal` class encapsulates zero or more connected slots (functors, methods, or functions)
	* and can emit events to all connected receivers.
	*
	* @tparam TARGS Argument types accepted by this signal.
	*/
	template<typename... TARGS>
	class signal
	{
	public:
		template<typename T> requires (priv::SISL_OBJECT<T>)
		signal(T& owner) : m_owner(owner)
		{}

		template<typename T> requires (!priv::SISL_OBJECT<T>)
			signal(T& owner)
		{ 
			static_assert(sizeof(TFUNCTOR) == 0, "[SISL] Signal ownership is restricted to types derived from " __SISL_STRINGIFY_DEFINE(SISL_NAMESPACE) "::object"); 
		}

		/**
		* @brief Connects a member function to this signal.
		*
		* @param instance Reference to the receiver object.
		* @param method Pointer to the member function.
		* @param type Connection type (default is automatic).
		*/
		template<typename TINSTANCE, typename TMETHOD> 
		requires priv::COMPATIBLE_METHOD_OF<TMETHOD, TINSTANCE, TARGS...>
		void connect(TINSTANCE& instance, TMETHOD method, type_connection type = type_connection::automatic)
		{
			// if the target instance is a sisl::generic_object, we may have more secure delegate to create
			if constexpr (std::is_base_of_v<priv::generic_object,TINSTANCE>)
			{
				// if we can get a weak_ptr on the instance, then the instance is managed by a shared_ptr
				// we can use a safe code for the delegate
				auto weak_instance = static_cast<priv::generic_object&>(instance).get_weak_ptr();
				if (!weak_instance.expired())
				{
					m_slots.emplace_back([weak_instance, method, type](TARGS... args)
					{
						if (auto shared_instance = weak_instance.lock())
						{
							(static_cast<TINSTANCE*>(shared_instance.get())->*method)(args...);
						}
					});
				}
				// otherwise we just call the method, no check
				else
				{
					m_slots.emplace_back([&instance, method, type](TARGS... args)
					{
						(instance.*method)(args...);
					});
				}
			}
			// otherwise we just call the method, no check
			else
			{
				m_slots.emplace_back([&instance, method, type](TARGS... args)
				{
					(instance.*method)(args...);
				});
			}
		}

		/**
		* @brief Connects a generic callable object (e.g. lambda, functor).
		*
		* @param functor The callable object.
		* @param type Connection type (default is automatic).
		*/
		template<typename TFUNCTOR>
		requires (priv::COMPATIBLE_FUNCTOR<TFUNCTOR, TARGS...>)
		void connect(TFUNCTOR&& functor, type_connection type = type_connection::automatic)
		{

		}

		/**
		 * @brief Connects a C/static function to the signal.
		 *
		 * @param function Pointer to the function.
		 * @param type Connection type (default is automatic).
		 */
		template<typename TFUNCTION>
		requires (priv::COMPATIBLE_FUNCTION<TFUNCTION, TARGS...>)
		void connect(TFUNCTION&& function, type_connection type = type_connection::automatic)
		{

		}

		template<typename TINSTANCE, typename TMETHOD>
		requires (!priv::COMPATIBLE_METHOD_OF<TMETHOD, TINSTANCE, TARGS...>)
		void connect(TINSTANCE&, TMETHOD, type_connection type = type_connection::automatic)
		{ 
			static_assert(sizeof(TINSTANCE) == 0, "[SISL] connect(): The provided method is not a member of the given object type or its argument types are incompatible with the signal's expected argument types.");
		}

		template<typename TFUNCTOR>
		requires (priv::is_functor_v<TFUNCTOR> && !priv::COMPATIBLE_FUNCTOR<TFUNCTOR, TARGS...>)
		void connect(TFUNCTOR&&, type_connection type = type_connection::automatic)
		{
			static_assert(sizeof(TFUNCTOR) == 0, "[SISL] connect(): The provided functor's argument types are incompatible with the signal's expected argument types.");
		}

		template<typename TFUNCTION>
		requires (std::is_function_v<std::remove_pointer_t<TFUNCTION>> && !priv::COMPATIBLE_FUNCTION<TFUNCTION, TARGS...>)
		void connect(TFUNCTION&&, type_connection type = type_connection::automatic)
		{
			static_assert(sizeof(TFUNCTION) == 0, "[SISL] connect(): The provided function's argument types are incompatible with the signal's expected argument types.");
		}

		void operator()() const requires (sizeof...(TARGS) == 0)
		{
			priv::gtl_current_sender = &m_owner;
			for (const auto& slot : m_slots)
			{
				slot();
			}
			priv::gtl_current_sender = nullptr;
		}

		void operator()(TARGS&&... args) const requires (sizeof...(TARGS) > 0)
		{
			priv::gtl_current_sender = &m_owner;
			for (const auto& slot : m_slots)
			{
				slot(args...);
			}
			priv::gtl_current_sender = nullptr;
		}
		
	private:
		priv::generic_object& m_owner;
		std::vector<std::function<void(TARGS...)>> m_slots;
	};
}

#endif //SISL_HPP