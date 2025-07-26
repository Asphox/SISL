#include <iostream>

#include "sisl.hpp"

class MyClass : public sisl::object<MyClass>
{
public:

	sisl_sig(on_clic, int);

	void test_i(int a) {
		std::cout << "sender !" << sisl::sender<MyClass>() << std::endl;
	}

	void test_d(double d) {
		std::cout << "sender !" << d << std::endl;
	}

	void test_string(std::string s)
	{
		std::cout << "sender !" << s << std::endl;
	}

	static void test_static(int a)
	{

	}
};

void test(std::string a)
{

}

int main()
{
	MyClass m;

	m.on_clic.connect(m, &MyClass::test_i);
	//m.on_clic.connect([](std::string a) {});
	emit m.on_clic(2);
	m.test_i(2);
	return 0;
}