#pragma once

#include "crypto/Sha256.h"
#include "p2p/Serialize.h"
#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <vector>

namespace Network
{
//! @ingroup NetworkGroup
//!@{

//! Represents an object in an inventory message
//!
//! Used in inventory requests for notifying other nodes about objects that they have or are requesting.
//!
//! @sa InventoryMessage

struct InventoryId : public P2p::Serializable
{
    //! Object type ID
    enum TypeId
    {
        TYPE_ERROR          = 0, //!< Any data of with this number may be ignored
        TYPE_TX             = 1, //!< Hash is related to a transaction
        TYPE_BLOCK          = 2, //!< Hash is related to a data block
        TYPE_FILTERED_BLOCK = 3 //!< Hash of a block header, but indicates that the reply should be a merkleblock message
    };

    // Constructor
    //!
    //! @param  type    type of inventory
    //! @param  hash    hash id of the inventory
    InventoryId(TypeId type, Crypto::Sha256Hash const & hash);

    // Deserializing Constructor
    //!
    //! @param[in,out]     in
    //! @param[in,out]     size
    //!
    //! @return
    InventoryId(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    TypeId type_;               //!< The ID of the type of the object
    Crypto::Sha256Hash hash_;   //!< A hash representing the object
};

//! A list of inventory IDs
typedef std::vector<InventoryId> InventoryList;

//!@}
} // namespace Network
