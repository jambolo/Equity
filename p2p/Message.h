#pragma once

#include "Serialize.h"
#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace P2p
{
//! @ingroup P2pGroup
//!@{

//! A P2P network message.
//!
//! @note	This is the raw form of a Network::Message.
//! @sa     https://en.bitcoin.it/wiki/Protocol_documentation#Message_structure

class Message
{
public:

    class Header;

    //! Contains a message's payload
    typedef std::vector<uint8_t> Payload;

    // Constructor
    //! @param  type        message type
    //! @param  payload     message payload
    Message(char const * type, std::vector<uint8_t> const & payload);

    //! Returns the message type
    std::string type() const { return type_; }

    //! Returns the payload
    Payload payload() const { return payload_;  }

    //! Returns the size of the payload
    size_t size() const { return payload_.size(); }

private:

    std::string type_;
    Payload payload_;
};

//! A P2P network message header.
//!
//! @sa     https://en.bitcoin.it/wiki/Protocol_documentation#Message_structure

class Message::Header : public P2p::Serializable
{
public:

    // Known magic numbers
    static uint32_t const MAGIC_MAIN  = 0xD9B4BEF9;     //!< Main network magic number
    static uint32_t const MAGIC_TEST  = 0xDAB5BFFA;     //!< Testnet network magic number
    static uint32_t const MAGIC_TEST3 = 0x0709110B;     //!< Testnet3 network magic number

    // Field sizes
    static size_t const MAGIC_SIZE    = 4;              //!< Size of the magic number field
    static size_t const TYPE_SIZE     = 12;             //!< Size of the type field
    static size_t const LENGTH_SIZE   = 4;              //!< Size of the length field
    static size_t const CHECKSUM_SIZE = 4;              //!< Size of the checksum field

    uint32_t magic_;                                    //!< Magic number
    char type_[TYPE_SIZE];                              //!< Message type
    uint32_t length_;                                   //!< Length of the payload
    uint32_t checksum_;                                 //!< Payload checksum

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}
};

//! Thrown by a message constructor if the data in the message is invalid.
struct InvalidMessageError : public std::runtime_error
{
    InvalidMessageError() : std::runtime_error("invalid message data") {}
};

//!@}
} // namespace Network
