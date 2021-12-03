#pragma once

#include "crypto/Sha256.h"
#include "p2p/Serialize.h"
#include <nlohmann/json_fwd.hpp>

namespace Equity
{
//! @addtogroup EquityGroup
//!@{

//! A transaction ID.
//!
//! This object is used to reference a transaction in the block chain.

struct Txid : public P2p::Serializable
{
    // Constructor
    Txid() {}

    // Constructor
    //! @param  json    A json representation of the txid
    explicit Txid(nlohmann::json const & json);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    Txid(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    Crypto::Sha256Hash hash_;   //!< The transaction ID in binary form
};

//!@}
} // namespace Equity
