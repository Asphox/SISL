/******************************************************************************
 * @file    sisl.hpp
 * @brief	
 *
 * @details Description plus détaillée si nécessaire, incluant :
 *          - le rôle général du fichier
 *          - les classes/fonctions principales définies ici
 *          - tout comportement ou usage particulier
 *
 * @author  SOTON "Asphox" Dylan
 * @date    2025-08-18
 * @version 1.0
 *
 * @note    Notes importantes (ex: dépendances, compatibilité, warnings)
 *
 * @copyright
 * MIT License
 * 
 * Copyright (c) [2025] [SOTON "Asphox" Dylan]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#ifndef SISL_HPP
#define SISL_HPP

// <=====================================================================================>
// <=============					Configuration							=============>
// <=====================================================================================>

// SISL needs C++20 or higher
#if __cplusplus < 202002L
#error "SISL requires C++20 or higher. Please enable C++20 support in your compiler settings."
#endif // __cplusplus < 202002L


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

/**
 * @def sisl_signal
 * @brief Semantic macro used to create a signal.
 *
 * @code
 * sisl_signal(on_click, int); // Defines a signal named on_click that takes an int parameter
 * @endcode
 */
#define sisl_signal __SISL_SIG_DEFINE

// <=====================================================================================>
// <=====================================================================================>
// <=====================================================================================>

// <=====================================================================================>
// <=============					 	Includes							=============>
// <=====================================================================================>
#include <optional>
#include <vector>
#include <functional>
#include <thread>
#include <future>
#include <queue>
#include <memory>
#include <variant>


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
	*/
	template<typename TSENDER>
	inline TSENDER* sender() { return priv::gtl_current_sender ? static_cast<TSENDER*>(priv::gtl_current_sender) : nullptr; }

	enum class polling_result : std::uint8_t
	{
		slots_invoked = 0,	///< At least one slot was invoked.
		timeout = 1,		///< The polling operation timed out without invoking any slots.
		terminated = 2,		///< The SISL polling mechanism was terminated, indicating that no further slots will be invoked.
	};

	constexpr std::chrono::milliseconds blocking_polling = std::chrono::milliseconds(INT64_MAX);

	/**
	 * @brief Polls the signal for any pending events.
	 *
	 * @param timeout Optional timeout for the polling operation(default is 0, meaning no wait / blocking_polling means wait indefinitely).
	 *
	 * This function processes all queued signals for the current thread and invokes connected slots accordingly.
	 * It is typically called in the main loop of a thread to allow multithreaded signal processing.
	 * 
	 * @return A polling_result indicating whether slots were invoked, the operation timed out, or the SISL mechanism was terminated.
	 */
	polling_result poll(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

	/**
	* @brief Terminates the SISL polling mechanism.
	* 
	* This function stops the SISL polling mechanism, preventing any further slots from being invoked in any thread.
	* It is typically called when the application is shutting down to unlock all blocking polling or polling with long timeout.
	* This operation is irreversible.
	*/
	void terminate_polling();

	/**
	* @class invalid_blocking_queued_connection
	* @brief Exception thrown when a blocking queued connection is attempted between a thread and itself.
	* 
	* This exception is used to indicate that a blocking queued connection cannot be established because it will lead to a deadlock.
	*/
	class invalid_blocking_queued_connection : public std::runtime_error
	{
	public:
		invalid_blocking_queued_connection()
			: std::runtime_error("Blocking queued connection cannot be established between a thread and itself. This would lead to a deadlock.") 
		{
		}
	};

	#define __SISL_SIG_DEFINE(name, ...) SISL_NAMESPACE##::signal<__VA_ARGS__> name{*this};
	#define __SISL_STR_DEFINE(x) #x
	#define __SISL_STRINGIFY_DEFINE(x) __SISL_STR_DEFINE(x)
	
	/**
	 * @enum type_connection
	 * @brief Defines the type of connection for signal-slot mechanisms.
	 *
	 * This enum specifies how a slot should be connected to a signal, determining the execution context and behavior.
	 */
	enum type_connection : std::uint8_t
	{
		automatic		= 0,			///< Automatically choose between direct and queued.
		direct			= 1,			///< Call slot immediately in the emitter's thread (ignores thread affinity).
		queued			= 2,			///< Enqueue slot to be invoked in the receiver's thread.
		blocking_queued	= 3,			///< Enqueue and block until the slot has finished.
		unique			= 1<<6,			///< Prevent multiple connections to the same slot.
		single_shot		= 1<<7,			///< Automatically disconnect after first trigger.
	};
	constexpr type_connection get_type_connection_without_flags(type_connection type) noexcept
	{
		return static_cast<type_connection>(static_cast<std::underlying_type_t<type_connection>>(type) & 0x3F);
	}
	
	namespace priv
	{
		template<typename T>
		concept has_weak_from_this = requires(T t) { { t.weak_from_this() }; };

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

		consteval std::thread::id get_empty_thread_id() noexcept { return std::thread::id(); }
		struct delegate_info
		{
			intptr_t		object;
			std::size_t		function;
			std::thread::id thread_affinity;
			type_connection	type;
		};

		enum class delegate_operation
		{
			call,			///< Call the delegate with the provided arguments.
			get_info,		///< Retrieve the delegate's information.
		};

		using delegate_return = std::variant<bool, std::reference_wrapper<const delegate_info>>;

		template<typename... TARGS>
		using delegate = std::function<delegate_return(delegate_operation, std::optional<std::tuple<TARGS...>>)>;

		template<typename... TARGS>
		class slot
		{
		public:
			slot(delegate<TARGS...>&& callee)
			{
				m_callee = std::move(callee);
			}

			const delegate_info& get_info() const
			{
				return std::get<std::reference_wrapper<const delegate_info>>(m_callee(delegate_operation::get_info, std::nullopt)).get();
			}

			inline bool operator()(TARGS&&... args) const
			{
				return std::get<bool>(m_callee(delegate_operation::call, std::make_tuple(std::forward<TARGS>(args)...)));
			}

		private:
			delegate<TARGS...> m_callee;
		};

		// Function to enqueue a delegate for execution in a specific thread
		void enqueue(std::function<void()>&& delegate, std::thread::id thread_id);
	}

	
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
		/**
		* @brief Constructs a signal with the specified owner.
		* @param owner Reference to the object that owns this signal.
		*/
		template<typename T>
		signal(T& owner) : m_owner(&owner)
		{}

		/**
		* @brief Connects a member function to this signal.
		*
		* @param instance Reference to the receiver object.
		* @param method Pointer to the member function.
		* @param thread_affinity Optional thread ID to specify the thread in which the slot should be executed (default will be the thread of emission).
		* @param type Connection type (default is automatic).
		*/
		template<typename TINSTANCE, typename TMETHOD> 
		requires priv::COMPATIBLE_METHOD_OF<TMETHOD, TINSTANCE, TARGS...>
		void connect(TINSTANCE& instance, TMETHOD method, std::thread::id thread_affinity = priv::get_empty_thread_id(), type_connection type = type_connection::automatic);

		/**
		* @brief Connects a generic callable object (e.g. lambda, functor).
		*
		* @param functor The callable object.
		* @param thread_affinity Optional thread ID to specify the thread in which the slot should be executed (default will be the thread of emission).
		* @param type Connection type (default is automatic).
		*/
		template<typename TFUNCTOR>
		requires (priv::COMPATIBLE_FUNCTOR<TFUNCTOR, TARGS...>)
		void connect(TFUNCTOR&& functor, std::thread::id thread_affinity = priv::get_empty_thread_id(), type_connection type = type_connection::automatic);

		/**
		 * @brief Connects a C/static function to the signal.
		 *
		 * @param function Pointer to the function.
		 * @param thread_affinity Optional thread ID to specify the thread in which the slot should be executed (default will be the thread of emission).
		 * @param type Connection type (default is automatic).
		 */
		template<typename TFUNCTION>
		requires (priv::COMPATIBLE_FUNCTION<TFUNCTION, TARGS...>)
		void connect(TFUNCTION&& function, std::thread::id thread_affinity = priv::get_empty_thread_id(), type_connection type = type_connection::automatic);

		/**
		* @brief Disconnects all slots connected to this signal.
		*/
		void disconnect_all();

		/**
		 * @brief Disconnects a specific slot from this signal.
		 *
		 * @param instance Reference to the object that owns the slot.
		 * @param method Pointer to the member function of the slot.
		 */
		template<typename TINSTANCE, typename TMETHOD>
		void disconnect(const TINSTANCE& instance, TMETHOD method);

		/**
		 * @brief Disconnects all slots connected to a specific instance.
		 *
		 * @param instance Reference to the object whose slots should be disconnected.
		 */
		template<typename TOBJECT>
		void disconnect(const TOBJECT& instance);

		/**
		 * @brief Disconnects all slots connected to a specific method.
		 *
		 * @param method Pointer to the member function of the slot.
		 */
		template<typename METHOD>
		void disconnect(METHOD method);

		/**
		 * @brief Invokes all connected slots with the provided arguments.
		 *
		 * This method calls each connected slot with the given arguments, removing any single-shot slots
		 * after they are invoked.
		 *
		 * @param args Arguments to pass to the connected slots.
		 */
		void operator()(TARGS&&... args);

		template<typename TINSTANCE, typename TMETHOD>
		requires (!priv::COMPATIBLE_METHOD_OF<TMETHOD, TINSTANCE, TARGS...>)
		void connect(TINSTANCE&, TMETHOD, std::thread::id, type_connection)
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
		
	private:
		void* m_owner;
		std::vector<priv::slot<TARGS...>> m_slots;
	};
}

// <=====================================================================================>
// <=============				Templates implementation					=============>
// <=====================================================================================>

namespace SISL_NAMESPACE
{
	template<typename... TARGS>
	template<typename TINSTANCE, typename TMETHOD>
	requires priv::COMPATIBLE_METHOD_OF<TMETHOD, TINSTANCE, TARGS...>
	void signal<TARGS...>::connect(TINSTANCE& instance, TMETHOD method, std::thread::id thread_affinity, type_connection type)
	{
		const priv::delegate_info info = { reinterpret_cast<intptr_t>(&instance), typeid(method).hash_code(), thread_affinity, type };
		if (type & type_connection::single_shot)
		{
			auto it = std::find_if(m_slots.begin(), m_slots.end(), [&info](const priv::slot<TARGS...>& slot)
			{
					return slot.get_info().object == info.object && slot.get_info().function == info.function;
			});
			if (it != m_slots.end())
			{
				// If the slot already exists, we do not add it again
				return;
			}
		}
		// if the target instance is a sisl::generic_object, we may have more secure delegate to create
		if constexpr (priv::has_weak_from_this<TINSTANCE>)
		{
			// if we can get a weak_ptr on the instance, then the instance is managed by a shared_ptr
			// we can use a safe code for the delegate
			auto weak_instance = instance.weak_from_this();
			if (!weak_instance.expired())
			{
				auto callee = [weak_instance, method, info](priv::delegate_operation operation, std::optional<std::tuple<TARGS...>> args) -> priv::delegate_return
				{
					if (operation == priv::delegate_operation::get_info)
					{
						return std::reference_wrapper<const priv::delegate_info>(info);
					}
					if (auto shared_instance = weak_instance.lock())
					{
						std::apply([&shared_instance, method](TARGS... args)
						{
							(static_cast<TINSTANCE*>(shared_instance.get())->*method)(args...);
						}, *args);
						return { true }; // Indicates successful call
					}
					else
					{
						return { false }; // Instance is no longer valid
					}
				};
				m_slots.emplace_back(std::move(callee));
			}
			// otherwise we just call the method, no check
			else
			{
				auto callee = [&instance, method, info](priv::delegate_operation operation, std::optional<std::tuple<TARGS...>> args)->priv::delegate_return
				{
					if (operation == priv::delegate_operation::get_info)
					{
						return std::reference_wrapper<const priv::delegate_info>(info);
					}
					std::apply([&instance, method](TARGS... args)
					{
						(instance.*method)(args...);
					}, *args);
					return { true }; // Indicates successful call
				};
				m_slots.emplace_back(std::move(callee));
			}
		}
		// otherwise we just call the method, no check
		else
		{
			auto callee = [&instance, method, info](priv::delegate_operation operation, std::optional<std::tuple<TARGS...>> args)->priv::delegate_return
			{
				if (operation == priv::delegate_operation::get_info)
				{
					return std::reference_wrapper<const priv::delegate_info>(info);
				}
				std::apply([&instance, method](TARGS... args)
				{
					(instance.*method)(args...);
				}, *args);
				return { true }; // Indicates successful call
			};
			m_slots.emplace_back(std::move(callee));
		}
	}

	template<typename... TARGS>
	template<typename TFUNCTOR>
	requires (priv::COMPATIBLE_FUNCTOR<TFUNCTOR, TARGS...>)
	void signal<TARGS...>::connect(TFUNCTOR&& functor, std::thread::id thread_affinity, type_connection type)
	{
		const priv::delegate_info info = { reinterpret_cast<intptr_t>(&functor), 0, thread_affinity, type };
		if (type & type_connection::single_shot)
		{
			auto it = std::find_if(m_slots.begin(), m_slots.end(), [&info](const priv::slot<TARGS...>& slot)
			{
				return slot.get_info().object == info.object && slot.get_info().function == 0;
			});
			if (it != m_slots.end())
			{
				// If the slot already exists, we do not add it again
				return;
			}
		}
		auto callee = [functor, info](priv::delegate_operation operation, std::optional<std::tuple<TARGS...>> args)->priv::delegate_return
		{
			if (operation == priv::delegate_operation::get_info)
			{
				return std::reference_wrapper<const priv::delegate_info>(info);
			}
			std::apply([&functor](TARGS... args)
			{
				functor(args...);
			}, *args);
			return { true }; // Indicates successful calls
		};
		m_slots.emplace_back(std::move(callee));
	}

	template<typename... TARGS>
	template<typename TFUNCTION>
	requires (priv::COMPATIBLE_FUNCTION<TFUNCTION, TARGS...>)
	void signal<TARGS...>::connect(TFUNCTION&& function, std::thread::id thread_affinity, type_connection type)
	{
		const priv::delegate_info info = { reinterpret_cast<intptr_t>(&function), 0, thread_affinity, type };
		if (type & type_connection::single_shot)
		{
			auto it = std::find_if(m_slots.begin(), m_slots.end(), [&info](const priv::slot<TARGS...>& slot)
			{
				return slot.get_info().object == info.object && slot.get_info().function == 0;
			});
			if (it != m_slots.end())
			{
				// If the slot already exists, we do not add it again
				return;
			}
		}
		auto callee = [function, info](priv::delegate_operation operation, std::optional<std::tuple<TARGS...>> args)->priv::delegate_return
		{
			if (operation == priv::delegate_operation::get_info)
			{
				return std::reference_wrapper<const priv::delegate_info>(info);
			}
			std::apply([&function](TARGS... args)
			{
				function(args...);
			}, *args);
			return { true }; // Indicates successful call
		};
		m_slots.emplace_back(std::move(callee));
	}
	
	template<typename... TARGS>
	void signal<TARGS...>::disconnect_all()
	{
		m_slots.clear();
	}

	template<typename... TARGS>
	template<typename TINSTANCE, typename TMETHOD>
	void signal<TARGS...>::disconnect(const TINSTANCE& instance, TMETHOD method)
	{
		std::erase_if(m_slots, [&instance, method](const priv::delegate<TARGS...>& slot)
		{
			return slot.m_id.object == reinterpret_cast<intptr_t>(&instance) && slot.m_id.function == typeid(method).hash_code();
		});
	}

	template<typename... TARGS>
	template<typename TOBJECT>
	void signal<TARGS...>::disconnect(const TOBJECT& instance)
	{
		std::erase_if(m_slots, [&instance](const priv::delegate<TARGS...>& slot)
		{
			return slot.m_id.object == reinterpret_cast<intptr_t>(&instance);
		});
	}

	template<typename... TARGS>
	template<typename METHOD>
	void signal<TARGS...>::disconnect(METHOD method)
	{
		std::erase_if(m_slots, [method](const priv::delegate<TARGS...>& slot)
		{
			return slot.m_id.function == typeid(method).hash_code();
		});
	}

	template<typename... TARGS>
	void signal<TARGS...>::operator()(TARGS&&... args)
	{
		const std::thread::id current_thread = std::this_thread::get_id();
		priv::gtl_current_sender = &m_owner;
		for (auto it = m_slots.begin(); it != m_slots.end(); )
		{
			const auto& slot = *it;
			const priv::delegate_info& info = slot.get_info();
			const type_connection type_without_flags = get_type_connection_without_flags(info.type);
			// Checks if the slot should be executed directly or queued
			const bool must_queue =		type_without_flags == type_connection::queued
									||	type_without_flags == type_connection::blocking_queued
									||	type_without_flags == type_connection::automatic && info.thread_affinity != priv::get_empty_thread_id() && info.thread_affinity != current_thread;
			
			bool result = true;
			if (must_queue)
			{
				const std::thread::id target_thread = info.thread_affinity == priv::get_empty_thread_id() ? current_thread : info.thread_affinity;
				// if the slot is blocking_queued, we need to wait for the slot to finish
				if (type_without_flags == type_connection::blocking_queued)
				{
					// If we are in the same thread, with blocking_queued, we MUST throw an exception because it would cause a deadlock.
					if (current_thread == target_thread)
					{
						m_slots.erase(it);
						throw invalid_blocking_queued_connection();
					}
					std::promise<void> done;
					auto future_done = done.get_future();
					priv::enqueue([slot, &done, ...args_capture = std::forward<TARGS>(args)]() mutable
					{
						try
						{
							slot(std::move(args_capture)...);
							done.set_value();
						}
						catch (...)
						{
							done.set_exception(std::current_exception());
						}
					}, target_thread);
					future_done.wait();
				}
				// If the slot is queued, we just enqueue it
				{
					priv::enqueue([slot, ...args_capture = std::forward<TARGS>(args)]() mutable
					{
						slot(std::move(args_capture)...);
					}, target_thread);
				}
			}
			// If the slot is direct, we call it directly
			else
			{
				result = slot(std::forward<TARGS>(args)...);
			}
			// If the slot is single-shot, remove it after calling
			if (!result || ((int)info.type & (int)type_connection::single_shot))
				it = m_slots.erase(it);
			else
				++it;
		}
		priv::gtl_current_sender = nullptr;
	}
}
// <=====================================================================================>
// <=============				  Statics implementation					=============>
// <=====================================================================================>

#ifdef SISL_IMPLEMENTATION

#include <condition_variable>
#include <mutex>

namespace SISL_NAMESPACE
{
	namespace priv
	{
		// The thread-local current sender.
		thread_local void* gtl_current_sender = nullptr;

		// A thread-safe queue for signals.
		// OPTIMIZATION: lock free queue ?
		struct signal_queue
		{
			std::queue<std::function<void()>> m_queue;
			std::condition_variable m_cv;
			std::mutex m_mutex;
		};

		// A thread-safe map of signal queues, indexed by thread ID.
		// Singleton pattern to ensure only one instance exists.
		struct hashmap_signal_queue
		{
			static hashmap_signal_queue& instance()
			{
				static hashmap_signal_queue instance;
				return instance;
			}

			signal_queue& get_thread_queue(std::thread::id thread_id)
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				auto it = m_queues.find(thread_id);
				if (it == m_queues.end())
				{
					auto [new_it, _] = m_queues.emplace(thread_id, std::make_unique<signal_queue>());
					return *new_it->second;
				}
				return *it->second;
			}

			const std::atomic_bool& terminated() const noexcept
			{
				return m_terminated;
			}

			void terminates()
			{
				m_terminated = true;
			}

			std::unordered_map<std::thread::id, std::unique_ptr<signal_queue>> m_queues;
			std::mutex m_mutex;
			std::atomic_bool m_terminated{ false }; ///< Flag to indicate if the SISL mechanism is terminated.
		};

		void enqueue(std::function<void()>&& delegate, std::thread::id thread_id)
		{
			auto& queue = hashmap_signal_queue::instance().get_thread_queue(thread_id);
			{
				std::lock_guard<std::mutex> lock(queue.m_mutex);
				queue.m_queue.push(std::move(delegate));
			}
			queue.m_cv.notify_one();
		}
	}

	polling_result poll(std::chrono::milliseconds timeout)
	{
		auto& queue = priv::hashmap_signal_queue::instance().get_thread_queue(std::this_thread::get_id());
		const auto& terminated = priv::hashmap_signal_queue::instance().terminated();
		if(terminated)
		{
			return polling_result::terminated; // If SISL is terminated, we return immediately.
		}
		std::unique_lock<std::mutex> lock(queue.m_mutex);
		if (timeout == blocking_polling)
		{
			queue.m_cv.wait(lock, [&queue, &terminated] { return !queue.m_queue.empty() || terminated; });
		}
		else if (timeout.count() > 0)
		{
			queue.m_cv.wait_for(lock, timeout, [&queue, &terminated] { return !queue.m_queue.empty() || terminated; });
		}
		if(queue.m_queue.empty())
		{
			return terminated ? polling_result::terminated : polling_result::timeout;
		}
		while (!queue.m_queue.empty())
		{
			auto delegate = std::move(queue.m_queue.front());
			queue.m_queue.pop();
			lock.unlock();
			delegate();
			lock.lock();
		}
		return terminated ? polling_result::terminated : polling_result::slots_invoked;
	}

	void terminate_polling()
	{
		priv::hashmap_signal_queue::instance().terminates();
	}
}

#endif //SISL_IMPLEMENTATION

#endif //SISL_HPP