#pragma once

#include "crypto/Sha256.h"

namespace Equity
{
//! @addtogroup EquityGroup
//!@{

//! Represents a target value in both 256-bit hash form and compact form.
//!
//!                            31                              0
//! Format of the compact form: eeeeeeeesmmmmmmmmmmmmmmmmmmmmmmm
//!
//! e is an 8-bit exponent (0 <= e <= 0x1d), s is a sign bit and m is a 23-bit unsigned mantissa (0 <= m <= 0x7fffff).
//! The value represented by the compact form is m * 256^(e-3). Note that since the target value is always positive, s
//! is always 0.

class Target
{
public:

    //! Target of difficulty 1 in compact form
    static uint32_t const DIFFICULTY_1_COMPACT = 0x1d00ffff;

    //! Target of difficulty 1 in hash form
    static Crypto::Sha256Hash const DIFFICULTY_1;

    //! Target value of 0 in compact form
    static uint32_t const TARGET_0_COMPACT = 0x0100ffff;

    // Constructor
    //!
    //! @param  hash     target in hash form
    explicit Target(Crypto::Sha256Hash const & hash);

    // Constructor
    //!
    //! @param compact  target in compact form
    explicit Target(uint32_t compact);

    //! Returns the hash form
    operator Crypto::Sha256Hash() const { return hash_; }

    //! Returns the compact form
    operator uint32_t() const { return compact_; }

    //! Returns the associated difficulty value
    double difficulty() const;

    //! Computes the hash form based on the compact form
    static Crypto::Sha256Hash convertToHash(uint32_t compact);

    //! Computes the compact form based on the hash form
    static uint32_t convertToCompact(Crypto::Sha256Hash const & hash);

private:

    static double toDouble(uint32_t compact);

    Crypto::Sha256Hash hash_;
    uint32_t compact_;
};

//! Less-than operator overload for Target
inline bool operator <(Target const & a, Target const b)
{
    // @BUG : Assumes that the compact form is normalized
    return uint32_t(a) < uint32_t(b);
}

//! Less-than-or-equal operator overload for Target
inline bool operator <=(Target const & a, Target const b)
{
    // @BUG : Assumes that the compact form is normalized
    return !(b < a);
}

//!@}
} // namespace Equity
