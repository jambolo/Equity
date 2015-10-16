#pragma once

#include <vector>
#include <cstdint>

#include <crypto/Sha256.h>

namespace Equity {

    class Transaction
    {
    public:
        Transaction();
        ~Transaction();

        std::vector<uint8_t> serialize();
        struct Input
        {
            Crypto::Sha256Hash transaction;
            uint32_t outputIndex;
            std::vector<uint8_t> script;
            uint32_t sequence;
        };
        typedef std::vector<Input> InputList;

        struct Output
        {
            uint64_t value;
            std::vector<uint8_t> script;
        };
        typedef std::vector<Output> OutputList;

        int version() const         { return version_; }
        InputList inputs() const    { return inputs_; }
        OutputList outputs() const  { return outputs_; }
        uint32_t lockTime() const   { return lockTime_; }

    private:

        int version_;
        InputList inputs_;
        OutputList outputs_;
        uint32_t lockTime_;
    };

}