#pragma once

#include "crypto/Sha256.h"
#include "equity/Transaction.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{

class Block
{
public:

    class Header
    {
public:
        int32_t version_;
        Crypto::Sha256Hash previousBlock_;
        Crypto::Sha256Hash merkleRoot_;
        uint32_t timestamp_;
        uint32_t target_;
        uint32_t nonce_;

        Header() {}
        Header(uint8_t const * & in, size_t & size);
        void serialize(std::vector<uint8_t> & out) const;

    };

    Block() {}
    Block(uint8_t const * & in, size_t & size);
    void serialize(std::vector<uint8_t> & out) const;

    std::string toJson() const;

    Header header() const { return header_; }
    TransactionList transactions() const { return transactions_; }

private:

    Header header_;
    TransactionList transactions_;
};

} // namespace Equity
