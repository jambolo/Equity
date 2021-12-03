#pragma once

#include <cassert>

//! @addtogroup UtilityGroup
//!@{

#if !defined(NDEBUG)
//! An assertion is thrown in a Debug build.
//!
//! In a Debug build, an assertion is thrown to indicate that the code has not yet been implemented. In a Release
//! build, nothing happens.
#define NOT_YET_IMPLEMENTED()                                                                    \
    _wassert(_CRT_WIDE(__FUNCTION__ " was called but is not implemented."), _CRT_WIDE(__FILE__), \
             (unsigned)(__LINE__))
//! An assertion is thrown in a Debug build.
//!
//! In a Debug build, an assertion is thrown to indicate that the code should never be executed. In a Release build,
//! nothing happens.
#define THIS_SHOULD_NEVER_HAPPEN()                                                  \
    _wassert(_CRT_WIDE("This code should never be executed."), _CRT_WIDE(__FILE__), \
             (unsigned)(__LINE__))
#else // if !defined(NDEBUG)
#define NOT_YET_IMPLEMENTED()
#define THIS_SHOULD_NEVER_HAPPEN()
#endif // if !defined(NDEBUG)

//!@}
