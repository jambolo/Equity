#pragma once

#include "crypto/Sha256.h"
#include "p2p/Serialize.h"

struct cJSON;

namespace Equity
{

//! A transaction ID.

struct Txid : public P2p::Serializable
{
    // Constructor
    Txid() {}

    // Constructor
    Txid(std::string const & json);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    Txid(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual cJSON_ptr toJson() const override;

    //!@}

    Crypto::Sha256Hash hash_;   //!< The transaction ID in binary form
};

} // namespace Equity
