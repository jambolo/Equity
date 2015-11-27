#pragma once

#include <cstdint>

namespace Network
{

struct Configuration
{
    uint32_t network;
    int port;

    static Configuration & get()  { return instance_; }

private:

    static Configuration instance_;
};

} // namespace Network
