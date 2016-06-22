#pragma once

#include <cassert>

#if !defined(NDEBUG)
  #define NOT_YET_IMPLEMENTED() _wassert(_CRT_WIDE(__FUNCTION__ " was called but is not implemented."), _CRT_WIDE(__FILE__), \
                                         (unsigned)(__LINE__))
  #define THIS_SHOULD_NEVER_HAPPEN() _wassert(_CRT_WIDE("This code should never be executed."), _CRT_WIDE(__FILE__), \
                                              (unsigned)(__LINE__))
#else
  #define NOT_YET_IMPLEMENTED()
  #define THIS_SHOULD_NEVER_HAPPEN()
#endif
