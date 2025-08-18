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

void thread_loop()
{
	while (sisl::poll(sisl::blocking_polling) != sisl::polling_result::terminated)
	{
		std::cout << "loop !" << std::endl;
	}
}


int main()
{
	std::thread t(thread_loop); // Start a thread that will poll for signals
	MyButton button1("MyButton1");
	MyWidget widget;
	button1.onClick.connect(widget, &MyWidget::onButtonClick, t.get_id(), sisl::type_connection::blocking_queued); // Connect the signal to the slot
	button1.onClick(2);
	button1.onClick(2);
	sisl::terminate();
	t.join();
	return 0;
}