#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace Network
{

//! @brief  Message base class
//!
//! All network message classes are derived from this class.

class Message
{
public:

    // Constructor
    Message(char const * type);

    //! Serializes the message
    virtual void serialize(std::vector<uint8_t> & out) const = 0;

    //! Returns the type
    std::string type() const { return type_; }

private:

    std::string type_;
};

//! Thrown by a message constructor if the data in the message is invalid.
struct InvalidMessageError : public std::runtime_error
{
    InvalidMessageError() : std::runtime_error("invalid message data") {}
};

} // namespace Network
