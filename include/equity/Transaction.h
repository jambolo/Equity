#pragma once

#include "Txid.h"
#include "crypto/Sha256.h"
#include "p2p/Serialize.h"
#include <cstdint>
#include <vector>

namespace Equity
{

class Transaction : public P2p::Serializable
{
public:

    //! The types of transactions. All but TYPE_Other are considered "standard"
    enum Type
    {
        TYPE_PayToPubkeyHash,
        TYPE_PayToScriptHash,
        TYPE_OpReturn,
        TYPE_Other
    };

    //! A transaction input
    struct Input : public P2p::Serializable
    {
        Txid txid;                      //!< Source transaction
        uint32_t outputIndex;           //!< Source transaction output index
        std::vector<uint8_t> script;    //!< Input script
        uint32_t sequence;              //!< Sequence number

        // Constructor
        Input()
        {
        }

        // Constructor
        //!
        //! @param  json    A json representation of the input
        explicit Input(json const & j);

        // Deserialization constructor
        //!
        //! @param[in,out]  in      pointer to the next byte to deserialize
        //! @param[in,out]  size    number of bytes remaining in the serialized stream
        Input(uint8_t const * & in, size_t & size);

        //! @name Overrides Serializable
        //!@{
        virtual void serialize(std::vector<uint8_t> & out) const override;
        virtual json toJson() const override;

        //!@}
    };

    //! A list of transactions inputs
    typedef std::vector<Input> InputList;

    //! A transaction output
    struct Output : public P2p::Serializable
    {
        uint64_t value;                 //!< Value of the output in satoshis
        std::vector<uint8_t> script;    //!< Output script

        // Constructor
        Output() {}

        // Constructor
        Output(std::string const & json);

        // Deserialization constructor
        //!
        //! @param[in,out]  in      pointer to the next byte to deserialize
        //! @param[in,out]  size    number of bytes remaining in the serialized stream
        Output(uint8_t const * & in, size_t & size);

        //! @name Overrides Serializable
        //!@{
        virtual void serialize(std::vector<uint8_t> & out) const override;
        virtual json toJson() const override;

        //!@}
    };

    //! A list of outputs
    typedef std::vector<Output> OutputList;

    // Constructor
    Transaction(int version, InputList const & inputs, OutputList const & outputs, uint32_t lockTime);

    // Constructor
    Transaction(std::string const & json);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    Transaction(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual json toJson() const override;

    //!@}

    uint32_t version() const { return version_; }
    InputList inputs() const { return inputs_; }
    OutputList outputs() const { return outputs_; }
    uint32_t lockTime() const { return lockTime_; }

    //! Returns true if the transaction is well-formed
    bool valid() const { return valid_; }

private:

    uint32_t version_;
    InputList inputs_;
    OutputList outputs_;
    uint32_t lockTime_;
    bool valid_;
};

//! A list of transactions
typedef std::vector<Transaction> TransactionList;

}
