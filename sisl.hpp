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
#include <queue>
#include <memory>
#include <variant>
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

	constexpr std::chrono::milliseconds blocking_polling = std::chrono::milliseconds(INT64_MAX);

	/**
	 * @brief Polls the signal for any pending events.
	 *
	 * @param timeout Optional timeout for the polling operation(default is 0, meaning no wait / blocking_polling means wait indefinitely).
	 *
	 * This method processes all queued signals for the current thread and invokes connected slots accordingly.
	 * It is typically called in the main loop of a thread to allow multithreaded signal processing.
	 */
	void poll(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

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
		[[nodiscard]] std::thread::id get_thread_id() override
		{
			return m_owner_thread;
		}

	private:
		std::thread::id m_owner_thread = std::this_thread::get_id();
	};

	/**
	* @enum type_connection
	* @brief Enumerates available connection policies for slots.
	*
	* These policies define how signals dispatch connected slots.
	²*/
	enum type_connection
	{
		automatic		= 0,			///< Automatically choose between direct and queued.
		direct			= 1,			///< Call slot immediately in the emitter's thread.
		queued			= 2,			///< Enqueue slot to be invoked in the receiver's thread.
		blocking_queued	= 3,			///< Enqueue and block until the slot has finished.
		unique			= 1<<16,		///< Prevent multiple connections to the same slot.
		single_shot		= 1<<17,		///< Automatically disconnect after first trigger.
	};
	
	namespace priv
	{
		struct delegate_info
		{
			intptr_t		object;
			std::size_t		function;
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
		template<typename T> requires (priv::SISL_OBJECT<T>)
		signal(T& owner) : m_owner(owner)
		{}

		/**
		* @brief Connects a member function to this signal.
		*
		* @param instance Reference to the receiver object.
		* @param method Pointer to the member function.
		* @param type Connection type (default is automatic).
		*/
		template<typename TINSTANCE, typename TMETHOD> 
		requires priv::COMPATIBLE_METHOD_OF<TMETHOD, TINSTANCE, TARGS...>
		void connect(TINSTANCE& instance, TMETHOD method, type_connection type = type_connection::automatic);

		/**
		* @brief Connects a generic callable object (e.g. lambda, functor).
		*
		* @param functor The callable object.
		* @param type Connection type (default is automatic).
		*/
		template<typename TFUNCTOR>
		requires (priv::COMPATIBLE_FUNCTOR<TFUNCTOR, TARGS...>)
		void connect(TFUNCTOR&& functor, type_connection type = type_connection::automatic);

		/**
		 * @brief Connects a C/static function to the signal.
		 *
		 * @param function Pointer to the function.
		 * @param type Connection type (default is automatic).
		 */
		template<typename TFUNCTION>
		requires (priv::COMPATIBLE_FUNCTION<TFUNCTION, TARGS...>)
		void connect(TFUNCTION&& function, type_connection type = type_connection::automatic);

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
		
		template<typename T> requires (!priv::SISL_OBJECT<T>)
		signal(T& owner)
		{
			static_assert(sizeof(TFUNCTOR) == 0, "[SISL] Signal ownership is restricted to types derived from " __SISL_STRINGIFY_DEFINE(SISL_NAMESPACE) "::object");
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
		
	private:
		priv::generic_object& m_owner;
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
	void signal<TARGS...>::connect(TINSTANCE& instance, TMETHOD method, type_connection type)
	{
		priv::delegate_info info = { reinterpret_cast<intptr_t>(&instance), typeid(method).hash_code(), type };
		// if the target instance is a sisl::generic_object, we may have more secure delegate to create
		if constexpr (std::is_base_of_v<priv::generic_object, TINSTANCE>)
		{
			// if we can get a weak_ptr on the instance, then the instance is managed by a shared_ptr
			// we can use a safe code for the delegate
			auto weak_instance = static_cast<priv::generic_object&>(instance).get_weak_ptr();
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
			auto callee = [&instance, method](priv::delegate_operation operation, std::optional<std::tuple<TARGS...>> args)->priv::delegate_return
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
	void signal<TARGS...>::connect(TFUNCTOR&& functor, type_connection type)
	{
		priv::delegate_info info = { reinterpret_cast<intptr_t>(&functor), 0, type };
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
	void signal<TARGS...>::connect(TFUNCTION&& function, type_connection type)
	{
		priv::delegate_info info = { reinterpret_cast<intptr_t>(&function), 0, type };
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
		priv::gtl_current_sender = &m_owner;
		for (auto it = m_slots.begin(); it != m_slots.end(); )
		{
			const auto& slot = *it;
			priv::delegate_info info = slot.get_info();
			const bool result = slot(std::forward<TARGS>(args)...);

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
		thread_local generic_object* gtl_current_sender = nullptr;

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

			std::unordered_map<std::thread::id, std::unique_ptr<signal_queue>> m_queues;
			std::mutex m_mutex;
		};
	}

	void poll(std::chrono::milliseconds timeout)
	{
		auto& queue = priv::hashmap_signal_queue::instance().get_thread_queue(std::this_thread::get_id());
		std::unique_lock<std::mutex> lock(queue.m_mutex);
		if (timeout == blocking_polling)
		{
			queue.m_cv.wait(lock, [&queue] { return !queue.m_queue.empty(); });
		}
		else if (timeout.count() > 0)
		{
			queue.m_cv.wait_for(lock, timeout, [&queue] { return !queue.m_queue.empty(); });
		}
		while (!queue.m_queue.empty())
		{
			auto delegate = std::move(queue.m_queue.front());
			queue.m_queue.pop();
			lock.unlock();
			delegate();
			lock.lock();
		}
	}
}

#endif //SISL_IMPLEMENTATION

#endif //SISL_HPP