#pragma once

#include "equity/Transaction.h"
#include "network/Message.h"

namespace Network
{

//! A transaction message.
//!
//! This message describes a bitcoin transaction, in reply to a get-data message.
//!
//! @sa GetDataMessage
//! @todo more to do for this message

class TransactionMessage : public Message
{
public:
    // Constructor
    TransactionMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    TransactionMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual json toJson() const override;

    //!@}

    int32_t version;                            //!< Transaction data format version(note, this is signed)
    Equity::Transaction::InputList inputs;      //!<  A list of 1 or more transaction inputs or sources for coins
    Equity::Transaction::OutputList outputs;    //!< A list of 1 or more transaction outputs or destinations for coins
    uint32_t lockTime;                          //!< The block number or timestamp at which this transaction is locked :
                                                //!<    Value       Description
                                                //!<    0	        Not locked
                                                //!< < 500000000	Block number at which this transaction is locked
                                                //!< >= 500000000	UNIX timestamp at which this transaction is locked
                                                //!< If all TxIn inputs have final (0xffffffff) sequence numbers then lockTime
                                                //!< is irrelevant. Otherwise, the transaction may not be added to a block
                                                //!< until after lock_time(see NLockTime).

    //! Message type
    static char const TYPE[];
};

} // namespace Network
