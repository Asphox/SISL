#include <iostream>

#define SISL_IMPLEMENTATION
#include "sisl.hpp"

class MyClass : public sisl::object<MyClass>
{
public:

	sisl_sig(on_clic, int);
	sisl_sig(onexit);

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

void test(int a)
{

}

void testv()
{
	std::cout << "testv" << std::endl;
}

int main()
{
	MyClass m;

	m.on_clic.connect(m, &MyClass::test_i);
	m.on_clic.connect([](int a) {});
	m.on_clic.connect(&test);
	m.onexit.connect(&testv);
	emit m.on_clic(2);
	m.onexit.disconnect_all();
	emit m.onexit();

	std::cout << sizeof(m.on_clic) << std::endl;

	m.test_i(2);
	return 0;
}