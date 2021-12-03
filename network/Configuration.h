#pragma once

#include <cstdint>

//! @defgroup NetworkGroup Bitcoin network layer
//!
//! The components of the Bitcoin network .

//! Bitcoin network layer.
//!
//! This library provides the functionality and components of the Bitcoin network layer needed to communicate with
//! other nodes using the inter-node communication protocol.
namespace Network
{
//! @ingroup NetworkGroup
//!@{

//! Configuration settings singleton for the Network module
struct Configuration
{
    uint32_t network;   //!< The bitcoin network magic number
    int port;           //!< The TCP/IP port to use

    //! Returns the configuration data instance
    static Configuration & get() { return instance_; }

private:

    static Configuration instance_;
};

//!@}
} // namespace Network
