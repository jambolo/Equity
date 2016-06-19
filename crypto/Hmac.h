#pragma once

#include "Sha512.h"

namespace Crypto
{

Sha512Hash hmacSha512(uint8_t const * key, size_t keySize, uint8_t const * message, size_t messageSize);

}

