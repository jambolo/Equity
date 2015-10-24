#pragma once

#include "crypto/Sha256.h"

namespace Equity {

    struct Txid : public Crypto::Sha256Hash
    {
        Txid() {}
        Txid(std::string const & json);
        Txid(uint8_t const *& in, size_t & size);

        void serialize(std::vector<uint8_t> & out) const;
        std::string toHex() const;
        std::string toJson() const;
    };

}