#pragma once

#include "Sha512.h"

namespace Crypto
{

//! Computes an HMAC of the message using SHA-512.
//!
//! @param  key             key
//! @param  keySize         size of the key
//! @param  message         message to generate the HMAC for
//! @param  messageSize     size of the message
Sha512Hash hmacSha512(uint8_t const * key, size_t keySize, uint8_t const * message, size_t messageSize);

}
