#pragma once

#include <vector>
#include <cstdint>

#include <crypto/Sha256.h>
#include "Txid.h"

namespace Equity {

    class Transaction
    {
    public:
        struct Input
        {
            Txid txid;
            uint32_t outputIndex;
            std::vector<uint8_t> script;
            uint32_t sequence;

            Input();
            Input(std::string const & json);
            Input(uint8_t const *& in, size_t & size);

            void serialize(std::vector<uint8_t> & out) const;
            std::string toHex() const;
            std::string toJson() const;
        };
        typedef std::vector<Input> InputList;

        struct Output
        {
            uint64_t value;
            std::vector<uint8_t> script;

            Output() {}
            Output(std::string const & json);
            Output(uint8_t const *& in, size_t & size);

            void serialize(std::vector<uint8_t> & out) const;
            std::string toHex() const;
            std::string toJson() const;
        };
        typedef std::vector<Output> OutputList;

        Transaction(int version, InputList const & inputs, OutputList const & outputs, uint32_t lockTime);
        Transaction(std::string const & json);
        Transaction(uint8_t const *& in, size_t & size);

        void serialize(std::vector<uint8_t> & out) const;
        std::string toHex() const;
        std::string toJson() const;

        int version() const         { return version_; }
        InputList inputs() const    { return inputs_; }
        OutputList outputs() const  { return outputs_; }
        uint32_t lockTime() const   { return lockTime_; }
        bool valid() const          { return valid_; }

    private:

        int version_;
        InputList inputs_;
        OutputList outputs_;
        uint32_t lockTime_;
        bool valid_;
    };

}