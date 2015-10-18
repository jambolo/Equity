#pragma once

#include "crypto/Sha256.h"

namespace Equity {

    struct Txid : public Crypto::Sha256Hash
    {
        Txid(std::string const & json);

        void serialize(std::vector<uint8_t> & out) const;
        std::string toHex() const;
        std::string toJson() const;
    };

}