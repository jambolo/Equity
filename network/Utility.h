#pragma once

#include "crypto/Sha256.h"
#include <cstdint>

namespace Network
{

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
    Crypto::Sha256Hash hash;
};
}
