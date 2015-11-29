#pragma once

#include "network/Message.h"

namespace Network
{

//! @brief Filter Load message
//!
//! Upon receiving a filterload command, the remote peer will immediately restrict the broadcast transactions it announces (in inv
//! packets) to transactions matching the filter, where the matching algorithm is specified below. The flags control the update
//! behaviour of the matching algorithm.

class FilterLoadMessage : public Message
{
public:

    FilterLoadMessage(std::vector<uint8_t> const & filter,
                      uint32_t                     nHashFuncs,
                      uint32_t                     tweak,
                      uint8_t                      flags);
    FilterLoadMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    std::vector<uint8_t> filter_;   //!< The filter itself is simply a bit field of arbitrary byte-aligned size. The maximum size is
                                    //!< 36,000 bytes.
    uint32_t nHashFuncs_;           //!< The number of hash functions to use in this filter. The maximum value allowed in this field
                                    //!< is 50.
    uint32_t tweak_;                //!< A random value to add to the seed value in the hash function used by the bloom filter.
    uint8_t flags_;                 //!< A set of flags that control how matched items are added to the filter.

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
