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