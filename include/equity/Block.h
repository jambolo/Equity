#pragma once

#include "crypto/Sha256.h"
#include "equity/Transaction.h"
#include "p2p/Serialize.h"
#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace Equity
{
//! A block in the block chain.
//!
//! A Block contains a list of validated transactions and information about its inclusion in the block chain

class Block : public P2p::Serializable
{
public:

    //! A standard block header.
    //!
    //! @note   The timestamp overflows in 2106
    //! @bug    In some cases, the version value is signed and in others it is unsigned

    struct Header : public P2p::Serializable
    {
        int32_t version;                    //!< Block version information, based upon the software version creating this block
        Crypto::Sha256Hash previousBlock;   //!< The hash value of the previous block this particular block references
        Crypto::Sha256Hash merkleRoot;      //!< The reference to a Merkle tree collection which is a hash of all transactions
        //!< related to this block
        uint32_t timestamp;                 //!< A timestamp recording when this block was created
        uint32_t target;                    //!< The calculated difficulty target being used for this block
        uint32_t nonce;                     //!< The nonce used to generate this block

        // Constructor
        Header() {}

        // Deserialization constructor
        //!
        //! @param[in,out]  in      pointer to the next byte to deserialize
        //! @param[in,out]  size    number of bytes remaining in the serialized stream
        Header(uint8_t const * & in, size_t & size);

        //! @name Overrides Serializable
        //!@{
        virtual void           serialize(std::vector<uint8_t> & out) const override;
        virtual nlohmann::json toJson() const override;

        //!@}
    };

    // Constructor
    Block() {}

    // Constructor
    //!
    //! @param  header          block header
    //! @param  transactions    transactions included in the block
    Block(Header const & header, TransactionList const & transactions);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    Block(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    //! Returns the header
    Header header() const { return header_; }

    //! Returns a list of transactions in the block
    TransactionList transactions() const { return transactions_; }

private:

    Header header_;
    TransactionList transactions_;
};

//! A list of blocks
typedef std::vector<Block> BlockList;

//! A list of block headers
typedef std::vector<Block::Header> BlockHeaderList;
} // namespace Equity
