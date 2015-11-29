#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace Network
{

//! @brief  Message base class
//!
//! All message classes are derived from this class.

class Message
{
public:

    static uint32_t const MAGIC_MAIN    = 0xF9BEB4D9;   //!< Main network magic number
    static uint32_t const MAGIC_TEST    = 0xFABFB5DA;   //!< Testnet network magic number
    static uint32_t const MAGIC_TEST3   = 0x0B110907;   //!< Testnet3 network magic number

    // Constructor
    Message(char const * command);

    //! Serializes the message
    virtual void serialize(std::vector<uint8_t> & out) const = 0;

    //! Returns the network magic number
    uint32_t magic() const { return magic_; }

    //! Returns the command string
    std::string command() const { return command_; }

    //! Returns the checksum (first 4 bytes of the double SHA-256 hash of the payload)
    uint32_t checksum() const { return checksum_; }

protected:

    //! Must be called by the derived class to serialize the entire message with the given payload
    void serialize(std::vector<uint8_t> const & payload, std::vector<uint8_t> & out) const;

private:

    size_t COMMAND_SIZE = 12;   // The command field is 12 bytes
    uint32_t magic_;
    std::string command_;
    uint32_t checksum_;
};

//! Thrown by a message constructor if the data in the message is invalid.
struct InvalidMessageError : public std::runtime_error
{
    InvalidMessageError() : std::runtime_error("invalid message data") {}
};

} // namespace Network
