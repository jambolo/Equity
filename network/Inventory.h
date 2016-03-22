#pragma once

#include "crypto/Sha256.h"
#include <cstdint>
#include <vector>

namespace Network
{

//! @brief  Represents an object
//!
//! Used in inventory requests for notifying other nodes about objects that they have or are requesting.
//!
//! @sa InventoryMessage

struct InventoryId
{
    //! Object type ID
    enum TypeId
    {
        TYPE_ERROR = 0,         //!< Any data of with this number may be ignored
        TYPE_TX = 1,            //!< Hash is related to a transaction
        TYPE_BLOCK = 2,         //!< Hash is related to a data block
        TYPE_FILTERED_BLOCK = 3 //!< Hash of a block header, but indicates that the reply should be a merkleblock message
    };

    InventoryId(TypeId type, Crypto::Sha256Hash const & hash);
    InventoryId(uint8_t const * & in, size_t & size);

    void serialize(std::vector<uint8_t> & out) const;

    TypeId type_;               //!< The ID of the type of the object
    Crypto::Sha256Hash hash_;   //!< A hash representing the object
};

typedef std::vector<InventoryId> InventoryList;

} // namespace Network