#pragma once

#include "network/Message.h"

namespace Network
{

//! A load-filter message.
//!
//! Upon receiving a load-filter message, the remote peer will immediately restrict the broadcast transactions it announces (in
//! inventory messages) to transactions matching the filter, where the matching algorithm is specified below. The flags control
//! the update behaviour of the matching algorithm.
//!
//! @note   This message is related to bloom filtering of connections and is defined in BIP 0037.
//! @sa     FilterAddMessage, FilterClearMessage, MerkleBlockMessage

class FilterLoadMessage : public Message
{
public:

    FilterLoadMessage(std::vector<uint8_t> const & filter,
                      uint32_t                     nHashFuncs,
                      uint32_t                     tweak,
                      uint8_t                      flags);
    FilterLoadMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    std::vector<uint8_t> filter_;   //!< The filter itself is simply a bit field of arbitrary byte-aligned size. The maximum size is
                                    //!< 36,000 bytes.
    uint32_t nHashFuncs_;           //!< The number of hash functions to use in this filter. The maximum value allowed in this field
                                    //!< is 50.
    uint32_t tweak_;                //!< A random value to add to the seed value in the hash function used by the bloom filter.
    uint8_t flags_;                 //!< A set of flags that control how matched items are added to the filter.

    //! Message type
    static char const TYPE[];
};

} // namespace Network
