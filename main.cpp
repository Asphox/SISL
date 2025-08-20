#include <iostream>
#define SISL_IMPLEMENTATION
//#define SISL_USE_LOCK_FREE_RING_QUEUE
//#define SISL_MAX_SLOTS_LOCK_FREE_RING_QUEUE 1024
#include "sisl.hpp"

static int g_counter = 0; // Global counter to demonstrate slot invocation

class CopyMe
{
public:
	CopyMe() = default;

	CopyMe(const CopyMe&) 
	{ 
		g_counter++;
	}
};

void func(CopyMe c)
{
	std::cout << "Button clicked! C" << std::endl;
}

struct TEST
{
	void func(CopyMe c)
	{
		std::cout << "Button clicked! C++" << std::endl;
	}
};


class MyButton
{
public: // Signals must be public
	sisl_signal(onClick, CopyMe); // Defines a signal named 'onClick' that carries

	MyButton(std::string&& name) : name(std::move(name)) {}
	std::string name; // Name of the button for identification
};

void thread_loop()
{
	while (sisl::poll(sisl::blocking_polling) != sisl::polling_result::terminated)
	{

	}
}

int main()
{
	std::thread t(thread_loop);
	MyButton button1("MyButton1");
	CopyMe c;
	sisl::connect(button1, &MyButton::onClick, [](CopyMe) {
		std::cout << "Button clicked! lambda" << std::endl;
	}, t.get_id());
	emit button1.onClick(c);
	std::cout << "copied :" << g_counter << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1)); // Give some time for the thread to process signals
	sisl::terminate();
	t.join();
	return 0;
}