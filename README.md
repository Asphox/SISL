# SISL

SISL is a modern single-header C++20 SIgnal/SLot library designed for type-safe, thread-aware event communication between objects. It provides a flexible API for connecting member functions, C-style functions, lambdas, and functors as slots, with advanced threading and connection policies.

# Inspirations

SISL is inspired by the Qt signal/slot mechanism, but aims to provide a more modern and type-safe implementation using C++20 features without the need for a preprocessor (Qt's MOC). It is designed to be lightweight, efficient, and easy to use in various new and existing C++ projects.

## Features

- **Configurable namespace**: All public symbols are under the `sisl` namespace (can be changed via `SISL_NAMESPACE` macro for integration purposes).
- **Type-safe signals**: Signals can carry any argument types and connect to slots with "static-compatibility" between signal and slot arguments.
- **Flexible connections**: Connect member functions, C-style functions, lambdas, or functors as slots.
- **Flexible disconnections**: Disconnect all slots, specific slots, or slots for a given instance or method.
- **Dangling pointer safety**: Automatically disconnect slots when the instance is destroyed, preventing dangling pointers (see 'dangling pointer safety' section).)
- **Threading support**: Signals can be emitted and processed in different threads, with connection policies (`direct`, `queued`, `blocking_queued`, etc.).
- **Single-shot and unique connections**: Support for auto-disconnect after first trigger and prevention of duplicate connections.

## How to use
To use SISL: 
1) Include the header 'sisl.hpp' in your project where needed:
```cpp
#include "sisl.hpp"
```
2) In ONE source file (.cpp), you MUST define the `SISL_IMPLEMENTATION` macro before including the header to enable implementation details:
```cpp
#define SISL_IMPLEMENTATION
#include "sisl.hpp"
```
3) Enjoy !

## Example of a simple signal-slot usage:
```cpp
#include <iostream>
#define SISL_IMPLEMENTATION
#include "sisl.hpp"
class MyButton 
{
public: // Signals must be public
	sisl_signal(onClick, int); // Defines a signal named 'onClick' that carries an int argument
};

class MyWidget 
{
public:
	void onButtonClick(int value) // Slot method that will be called when the signal is emitted
	{
		std::cout << "Button clicked with value: " << value << std::endl;
	}
};

int main() 
{
	MyButton button;
	MyWidget widget;
	// Connect the signal to the slot
	button.onClick.connect(widget, &MyWidget::onButtonClick);
	// Emit the signal with an integer value
	emit button.onClick(42); // This will call widget.onButtonClick(42)
	return 0;
}
```

## How can I know who emitted the signal inside a slot?
You can use the `sisl::sender<T>()` function inside a slot to get a pointer to the object that emitted the signal. This is useful for identifying the sender when multiple objects can emit the same signal.
```cpp
#include <iostream>
#define SISL_IMPLEMENTATION
#include "sisl.hpp"
class MyButton
{
public: // Signals must be public
	sisl_signal(onClick, int); // Defines a signal named 'onClick' that carries

	MyButton(std::string&& name) : name(std::move(name)) {}
	std::string name; // Name of the button for identification
};

class MyWidget
{
public:
	void onButtonClick(int value) // Slot method that will be called when the signal is emitted
	{
		auto sender = sisl::sender<MyButton>(); // Get the sender of the signal
		if (sender)
		{
			std::cout << "Button clicked with value: " << value << " from button: " << sender->name << std::endl;
		}
	}
};

int main()
{
	MyButton button1("MyButton1");
	MyButton button2("MyButton2");
	MyWidget widget;
	// Connect the signal to the slot
	button1.onClick.connect(widget, &MyWidget::onButtonClick);
	button2.onClick.connect(widget, &MyWidget::onButtonClick);
	// Emit the signal with an integer value
	emit button1.onClick(42); // This will call widget.onButtonClick(42) and print the sender's name
	emit button2.onClick(42); // This will call widget.onButtonClick(42) and print the sender's name
	return 0;
}
```


## Dangling pointer safety
SISL can automatically disconnects slots when the instance they are connected to is destroyed. This prevents dangling pointers and ensures that slots do not attempt to access invalid memory.
To enable this feature, a slot owner **MUST** be managed by std::shared_ptr **AND** inherit from std::enable_shared_from_this.
```cpp
#include <iostream>
#include <memory>
#define SISL_IMPLEMENTATION
#include "sisl.hpp"
class MyButton
{
public: // Signals must be public
	sisl_signal(onClick, int); // Defines a signal named 'onClick' that carries an int argument
};

class MyWidget : public std::enable_shared_from_this<MyWidget> // Inherits from std::enable_shared_from_this
{
public:
	void onButtonClick(int value) // Slot method that will be called when the signal is emitted
	{
		std::cout << "Button clicked with value: " << value << std::endl;
	}
};

int main()
{
	MyButton button;
	{
		auto managed_widget = std::make_shared<MyWidget>(); // This widget is managed by shared_ptr, so it will automatically disconnect when destroyed.
		MyWidget unmanaged_widget; // This widget is not managed by shared_ptr, so it will not disconnect automatically, and cause dangling pointer issues.
		// Connect the signal to the slot
		button.onClick.connect(managed_widget, &MyWidget::onButtonClick);
		button.onClick.connect(unmanaged_widget, &MyWidget::onButtonClick); // This will cause dangling pointer issues if 'unmanaged_widget' is destroyed before 'button'.
	} // managed_widget and unmanaged_widget go out of scope here, managed_widget will be destroyed and disconnected, but unmanaged_widget will not.
	
	// This will call the dangling unmanaged_widget.onButtonClick(42) (and hopefully crash !), but not unmanaged_widget.onButtonClick(42) since it was destroyed !
	emit button.onClick(42);
	return 0;
}
```

## Threading and connection policies
SISL supports different threading models and connection policies for signal-slot connections. The `type_connection` enum defines how slots are connected and executed:
```cpp
enum class type_connection {
	automatic,          // Automatically chooses between direct and queued
	direct,             // Calls slot immediately in emitter's thread
	queued,             // Enqueues slot for execution in receiver's thread
	blocking_queued,    // Enqueues and blocks until slot finishes
	unique,             // Prevents multiple connections to the same slot
	single_shot         // Disconnects after first trigger
};
```
Here an example of how to use different connection policies with threads:
```cpp
#include <iostream>
#include <thread>
#define SISL_IMPLEMENTATION
#include "sisl.hpp"
class MyButton
{
public: // Signals must be public
	sisl_signal(onClick, int); // Defines a signal named 'onClick' that carries
};

class MyWidget : public std::enable_shared_from_this<MyWidget>
{
public:
	void onButtonClick(int value) // Slot method that will be called when the signal is emitted
	{
		std::cout << "Button clicked with value: " << value << " in the thread: " << std::this_thread::get_id() << std::endl;
	}
};

// Example of a thread that will process signals
void thread_loop()
{
	while (sisl::poll(std::chrono::milliseconds(1000)) != sisl::polling_result::terminated) // Poll for pending signals every second
	{
		std::cout << "thread loop" << std::endl;
	}
}

int main()
{
	MyButton button;
	auto managed_widget = std::make_shared<MyWidget>();
	std::thread worker(thread_loop); // Start a thread to process signals
	const auto thread_id = worker.get_id(); // Get the thread ID of the worker thread
	// Connect the signal to the slot with different connection policies
	button.onClick.connect(*managed_widget.get(), &MyWidget::onButtonClick, thread_id, sisl::type_connection::direct); // Direct connection (thread_id is ignored, called immediately in the emitter's thread)
	button.onClick.connect(*managed_widget.get(), &MyWidget::onButtonClick, thread_id, sisl::type_connection::queued); // Queued connection
	button.onClick.connect(*managed_widget.get(), &MyWidget::onButtonClick, thread_id, sisl::type_connection::blocking_queued); // Blocking queued connection
	// Emit the signal with an integer value
	emit button.onClick(42); // This will call managed_widget.onButtonClick(42) in the appropriate thread based on connection policy

	sisl::terminate(); // Stop polling in all threads
	worker.join(); // Wait for the worker thread to finish
	return 0;
}
```
## SISL API Documentation

### Class `sisl::signal<TARGS...>`

Represents a typed signal.  
Example: `sisl::signal<int>` for a signal carrying an integer.

#### Main methods

- `connect(instance, &Class::method, thread_id, type_connection)`  
  Connects a member function as a slot.
- `connect(functor, thread_id, type_connection)`  
  Connects a functor or lambda.
- `connect(function, thread_id, type_connection)`  
  Connects a free or static function.
- `disconnect_all()`  
  Disconnects all slots.
- `disconnect(instance, method)`  
  Disconnects a specific slot.
- `disconnect(instance)`  
  Disconnects all slots for an instance.
- `disconnect(method)`  
  Disconnects all slots for a method.
- `operator()(args...)`  
  Emits the signal with the given arguments.

### Enum `sisl::type_connection`

Defines connection policy:
- `automatic`: Automatically chooses between direct and queued.
- `direct`: Calls slot immediately in emitter's thread.
- `queued`: Enqueues slot for execution in receiver's thread.
- `blocking_queued`: Enqueues and blocks until slot finishes.
- `unique`: Prevents multiple connections to the same slot.
- `single_shot`: Disconnects after first trigger.

### Enum `sisl::polling_result`
Defines the result of polling for signals:
- `slots_invoked`: At least one slot was invoked.
- `timeout`: The polling operation timed out without invoking any slots.
- `terminated`: The SISL polling mechanism was terminated, indicating that no further slots will be invoked.

### Function `sisl::poll(timeout)`

Processes pending signals for the current thread.  
If `timeout` is provided, it specifies the maximum time to wait for signals. If timeout is set to sisl::blocking_polling, it will block indefinitely until a signal is emitted or polling is terminated.
Typically called in a thread's main loop.
Returns `sisl::polling_result` indicating whether any slots were invoked, if the operation timed out, or if polling was terminated.

### Function `sisl::terminate()`
Terminates the SISL polling mechanism, stopping all threads that are currently polling for signals.
This is useful for gracefully shutting down the SISL system when it is no longer needed.

### Function `sisl::sender<T>()`

Returns a pointer to the object that emitted the currently executing signal (inside a slot).
This is useful for identifying the sender when multiple objects can emit the same signal.
Returns `nullptr` if called outside a slot.

### For more documentation
See doxygen comments in the source code.

### Disclaimer
This library is currently in development and may change in future versions.
Some optimizations will be added in the future, such as a lock-free queue for queued connections and more efficient memory management.
Please report any issues or suggestions on the GitHub repository.
