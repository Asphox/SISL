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

##Tutorial

First, include the Sisl header.
```cpp
#include "SISL/Sisl.hpp"
```

#How to create a signal ?

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
