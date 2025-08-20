#include <iostream>
#define SISL_IMPLEMENTATION
//#define SISL_USE_LOCK_FREE_RING_QUEUE
//#define SISL_MAX_SLOTS_LOCK_FREE_RING_QUEUE 1024
#include "sisl.hpp"
class MyButton
{
public: // Signals must be public
	sisl_signal(onClick, int); // Defines a signal named 'onClick' that carries

	MyButton(std::string&& name) : name(std::move(name)) {}
	std::string name; // Name of the button for identification
};

static int v = 0;

class MyWidget
{
public:
	void onButtonClick(int value) // Slot method that will be called when the signal is emitted
	{
		auto sender = sisl::sender<MyButton>(); // Get the sender of the signal
		if (sender)
		{
			std::cout << "Button clicked with value: " << value << " from button: " << sender->name << "   " << v++ << std::endl;
		}
	}
};

void thread_loop()
{
	while (sisl::poll(sisl::blocking_polling) != sisl::polling_result::terminated)
	{
		std::cout << "loop !" << std::endl;
	}
}

void test(int a)
{
	std::cout << "test " << a << std::endl;
}

int main()
{
	std::thread t(thread_loop); // Start a thread that will poll for signals
	MyButton button1("MyButton1");
	MyWidget widget;
	for(int i = 0; i < 10000; ++i)
	{
		sisl::connect(button1, &MyButton::onClick, widget, &MyWidget::onButtonClick);
		sisl::connect(button1, &MyButton::onClick, [](int a) { std::cout << "pouet " << a << std::endl; }); // <== erreur !
		sisl::connect(button1, &MyButton::onClick, &test);
	}
	emit button1.onClick(2);
	std::this_thread::sleep_for(std::chrono::seconds(1)); // Give some time for the thread to process signals
	sisl::terminate();
	t.join();
	return 0;
}