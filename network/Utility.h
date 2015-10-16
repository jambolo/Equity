#pragma once

#include <cstdint>
#include "crypto/Sha256.h"

namespace Network
{

    struct Address
    {
        uint32_t time;
        uint64_t services;
        char ip[16];
        uint16_t port;
    };

    struct Inventory
    {
        enum ObjectTypeId
        {
            OBJECT_ERROR = 0,
            OBJECT_TX = 1,
            OBJECT_BLOCK = 2,
            OBJECT_FILTERED_BLOCK = 3
        };
        uint32_t type;
        Crypto::Sha256Hash  hash;
    };
}