#pragma once

#include <cstdint>
#include <vector>

#include "Txid.h"
#include <crypto/Sha256.h>

namespace Equity
{

class Transaction
{
public:

    enum Type
    {
        TYPE_PayToPubkeyHash,
        TYPE_PayToScriptHash,
        TYPE_OpReturn,
        TYPE_Other
    };

    struct Input
    {
        Txid txid;
        uint32_t outputIndex;
        std::vector<uint8_t> script;
        uint32_t sequence;

        Input();
        Input(std::string const & json);
        Input(uint8_t const * & in, size_t & size);

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
        Output(uint8_t const * & in, size_t & size);

        void serialize(std::vector<uint8_t> & out) const;
        std::string toHex() const;
        std::string toJson() const;

    };
    typedef std::vector<Output> OutputList;

    Transaction(int version, InputList const & inputs, OutputList const & outputs, uint32_t lockTime);
    Transaction(std::string const & json);
    Transaction(uint8_t const * & in, size_t & size);

    void serialize(std::vector<uint8_t> & out) const;
    std::string toHex() const;
    std::string toJson() const;

    int version() const { return version_; }
    InputList inputs() const { return inputs_; }
    OutputList outputs() const { return outputs_; }
    uint32_t lockTime() const { return lockTime_; }
    bool valid() const { return valid_; }

private:

    int version_;
    InputList inputs_;
    OutputList outputs_;
    uint32_t lockTime_;
    bool valid_;
};

//! List of transactions
typedef std::vector<Transaction> TransactionList;

}
