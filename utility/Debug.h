#pragma once

#include <cassert>

//! @addtogroup UtilityGroup
//!@{

#if !defined(NDEBUG)
//! An assertion is thrown in a Debug build.
//!
//! In a Debug build, an assertion is thrown to indicate that the code has not yet been implemented. In a Release
//! build, nothing happens.
#define NOT_YET_IMPLEMENTED() assert(!" Function was called but is not implemented.")
//! An assertion is thrown in a Debug build.
//!
//! In a Debug build, an assertion is thrown to indicate that the code should never be executed. In a Release build,
//! nothing happens.
#define THIS_SHOULD_NEVER_HAPPEN() assert(!"This code should never be executed.")
#else // if !defined(NDEBUG)
#define NOT_YET_IMPLEMENTED()
#define THIS_SHOULD_NEVER_HAPPEN()
#endif // if !defined(NDEBUG)

//!@}
