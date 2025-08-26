#include <iostream>
#define SISL_IMPLEMENTATION
#define SISL_USE_LOCK_FREE_RING_QUEUE
#define SISL_MAX_SLOTS_LOCK_FREE_RING_QUEUE 1024
#include "sisl.hpp"

static int g_counter = 0; // Global counter to demonstrate slot invocation
static int g_move = 0;

class CopyMe
{
public:
	CopyMe() = default;

	CopyMe(const CopyMe&) 
	{ 
		g_counter++;
	}
	CopyMe(CopyMe&&) noexcept
	{
		g_move++;
	}
};

void func(CopyMe c)
{
	std::cout << "Button clicked! C" << std::endl;
}

struct TEST
{
	void func(CopyMe c2, CopyMe&)
	{
		std::cout << "Button clicked! C++" << std::endl;
	}
};


class MyButton
{
public: // Signals must be public
	sisl_signal(onClick, CopyMe, CopyMe&); // Defines a signal named 'onClick' that carries

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
	TEST test;
	std::thread t(thread_loop);
	MyButton button1("MyButton1");
	CopyMe c;
	sisl::connect(button1, &MyButton::onClick, test, &TEST::func, t.get_id());
	//sisl::connect(button1, &MyButton::onClick, [](CopyMe s) { std::cout << "Button clicked! lambda" << std::endl; (void)s; }, t.get_id());
	emit button1.onClick(c, c);
	std::this_thread::sleep_for(std::chrono::seconds(1)); // Give some time for the thread to process signals
	sisl::terminate();
	std::cout << "copied :" << g_counter << std::endl;
	std::cout << "moved :" << g_move << std::endl;
	t.join();
	return 0;
}