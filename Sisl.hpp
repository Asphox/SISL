////////////////////////////////////////////////////////////
//
// SISL - An other SIgnals SLots library
// Copyright (C) 2017-2018 SOTON "Asphox" Dylan (dylan.soton@telecom-sudparis.eu)
//
////////////////////////////////////////////////////////////

#ifndef SISL_HPP
#define SISL_HPP

#define DEBUG false

#if __cplusplus > 199711L

#include "headers/Signal.hpp"

/*================================
Macro for member signals
==================================*/
#define SIGNAL(name,...)  sisl::Signal<__VA_ARGS__> name = sisl::Signal<__VA_ARGS__>(this)

/*================================
Macro for static signals
==================================*/
#define SSIGNAL(name,...) sisl::Signal<__VA_ARGS__> name;

/*================================
Void macro for emit readability
==================================*/

#else
#error [SISL] YOU NEED TO SET YOUR COMPILER WITH ISO C++11 OR MORE !
#endif

#endif //SISL_HPP
