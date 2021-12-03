#pragma once

#include "p2p/Serialize.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace Network
{
//! @ingroup NetworkGroup
//!@{

//! Base class for all network message classes.

class Message : public P2p::Serializable
{
public:

    // Constructor
    //! @param  type    string used to identify the message type during deserialization
    explicit Message(char const * type);

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

//!@}
} // namespace Network
