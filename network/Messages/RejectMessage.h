#pragma once

#include "crypto/Sha256.h"
#include "network/Message.h"

namespace Network
{

//! A reject message.
//!
//! The reject message is sent when messages are rejected. It is sent in response to a version, tx or block message.
//!
//! @sa https://github.com/bitcoin/bips/blob/master/bip-0061.mediawiki

class RejectMessage : public Message
{
public:

    //! Values describing the reasons for a rejection
    enum Code
    {
        REJECT_MALFORMED        = 0x01,
        REJECT_INVALID          = 0x10,
        REJECT_OBSOLETE         = 0x11,
        REJECT_DUPLICATE        = 0x12,
        REJECT_NONSTANDARD      = 0x40,
        REJECT_DUST             = 0x41,
        REJECT_INSUFFICIENTFEE  = 0x42,
        REJECT_CHECKPOINT       = 0x43
    };

    // Constructor
    //!
    //! @param  message     Type of message rejected
    //! @param  code        Code relating to rejected message
    //! @param  reason      Text version of reason for rejection
    //! @param  data        Optional extra data provided by some errors. Currently, all errors which provide this
    //!                     field fill it with the TXID or block header hash of the object being rejected, so the
    //!                     field is 32 bytes.
    RejectMessage(std::string const &          message,
                  Code                         code,
                  std::string const &          reason,
                  std::vector<uint8_t> const & data);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    RejectMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual json toJson() const override;

    //!@}

    std::string message_;           //!< The message type of the rejected message
    uint8_t code_;                  //!< The rejection code
    std::string reason_;            //!< The reason for the rejection
    std::vector<uint8_t> data_;     //!< Optional data related to the rejection

    //! Message type
    static char const TYPE[];
};

} // namespace Network
