#pragma once
// should be ported to use contracts in future, if ever contracts can be part of c++
#if defined(NO_ASSERT)
#define SPA_ASSERT(x)
#else
#include <cassert>
#define SPA_ASSERT(x) assert(!!(x));
#endif
