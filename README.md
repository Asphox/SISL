# SISL
An other cross-platform/cross-compiler Signals and Slots library for C++11 and more...

SISL is core-based on the C++ delegate implementation of Quynh Nguyen from https://www.codeproject.com/Articles/18886/A-new-way-to-implement-Delegate-in-C (2007).

advantages:

-cross platfrom

-cross compiler

-plain C++11 code

-flexible

-full headers library

-dangling safe ( you can't call a Delegate on a member function from a destroyed object )

...

## Tutorial

First, include the Sisl header.
```cpp
#include "SISL/Sisl.hpp"
```

### How to create a signal ?

The standard way :

```cpp
//inside a class
public:
sisl::Signal<ARGS> mySignal = sisl::Signal<ARGS>(this);

//outside a class
sisl::Signal<ARGS> mySignal;
```

or with macros :

```cpp
//inside a class
public:
SIGNAL(mySignal,ARGS);

//outside a class
SSIGNAL(mySignal,ARGS);
```
### How to use and activate <ADVANCED> functionalites of SISL ?

Just inherit from sisl::SislObject in your classes.

```cpp
class MyClass : public sisl::SislObject
{

};
```

### How to connect functions or functors to a signal ?

By using the connect method of the signal, you can connect :
-static functions ( C-style )
-member functions ( normal, const or/and volatile )
-std::function objects
-lambda functions

<!>WARNING<!> win32 special calling conventions are not supporter yet !

The only restriction is that the arguments of the signal and the function must match.

```cpp
//static function
mySignal.connect(&myStaticFunction);

//member function
mySignal.connect(&instance,&Class::myMemberFunction);

//std::function
std::function<F> stdFunction;
mySignal.connect(stdFunction);

//lambda function
mySignal.connect([](){});
```

### How to emit a signal ?

By using the emit method or the () operator of Signal.
Don't forget to pass parameters if the signal must emit data.
```cpp
mySignal.emit(PARAMETERS);
//or
mySignal(PARAMETERS);
//or for readability
emit mySignal(PARAMETERS);
```

### How to disconnect a function of a signal ?

By using the disconnect functions.
<!>WARNING<!> You can't disconnect std::functions and lambda functions except with disconnect_all() !

```cpp
//disconnect static function
mySignal.disconnect(&myStaticFunction);

//disconnect member function
mySignal.disconnect(&instance,&Class::myMemberFunction);

//disconnect all member functions owned by an instance
mySignal.disconnect_all(&instance);

//disconnect this specific member function from all instances
mySignal.disconnect_all(&Class::myMemberFunction);

//disconnect ... all
mySignal.disconnect_all();

```

### <ADVANCED> How to retrieve the address of the emitting object (only for member signals) ?

By using the getSender<TYPE>() method of the SislObject class :

```cpp
class MyClass : public sisl::SislObject
{
  public:
    void function(PARAMETERS)
    {
      getSender<SENDER_TYPE>()->someFunction();
    }
};
```
