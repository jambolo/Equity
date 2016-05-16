#pragma once

#include <cstdint>

namespace Network
{

//! Configuration settings singleton for the Network module
struct Configuration
{
    uint32_t network;   //!< The bitcoin network magic number
    int port;           //!< The TCP/IP port to use

    //! Returns the configuration data instance
    static Configuration & get()  { return instance_; }

private:

    static Configuration instance_;
};

} // namespace Network
