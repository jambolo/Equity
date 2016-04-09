#pragma once

#include "crypto/Sha256.h"
#include "p2p/Serialize.h"

namespace Equity
{

struct Txid : public P2p::Serializable
{
    Txid() {}
    Txid(std::string const & json);
    Txid(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;
    std::string toHex() const;
    std::string toJson() const;

    Crypto::Sha256Hash hash_;
};

} // namespace Equity
