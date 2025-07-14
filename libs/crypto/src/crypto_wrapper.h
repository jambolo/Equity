#pragma once

#include <cstdint>
#include <cstddef>
#include "rust/cxx.h"

// C wrapper function declarations for cxx bridge compatibility

extern "C" {

// SHA-1 wrapper
void sha1(const uint8_t* input, size_t length, uint8_t* output);

// SHA-256 wrappers
void sha256(const uint8_t* input, size_t length, uint8_t* output);
void doubleSha256(const uint8_t* input, size_t length, uint8_t* output);
void checksum(const uint8_t* input, size_t length, uint8_t* output);

// SHA-512 wrapper
void sha512(const uint8_t* input, size_t length, uint8_t* output);

// RIPEMD-160 wrapper
void ripemd160(const uint8_t* input, size_t length, uint8_t* output);

// HMAC-SHA512 wrapper
void hmacSha512(const uint8_t* key, size_t keySize, const uint8_t* message, size_t messageSize, uint8_t* output);

// PBKDF2 wrapper - returns success/failure and fills output buffer
bool pbkdf2HmacSha512(const uint8_t* password, size_t passwordSize, 
                      const uint8_t* salt, size_t saltSize, 
                      int count, size_t outputSize, uint8_t* output);

// ECC wrappers
bool eccPublicKeyIsValid(const uint8_t* k, size_t size);
bool eccPrivateKeyIsValid(const uint8_t* k, size_t size);

// NEW: ECC key derivation wrapper
bool eccDerivePublicKey(const uint8_t* privateKey, rust::Vec<uint8_t>& publicKey, bool uncompressed);

// NEW: ECC signing wrapper  
bool eccSign(const uint8_t* message, size_t messageSize, const uint8_t* privateKey, rust::Vec<uint8_t>& signature);

// NEW: ECC verification wrapper
bool eccVerify(const uint8_t* message, size_t messageSize, const rust::Vec<uint8_t>& publicKey, const rust::Vec<uint8_t>& signature);

// NEW: Vector-based hash functions for convenience
void sha1Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output);
void sha256Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output);
void doubleSha256Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output);
void checksumVector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output);
void sha512Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output);
void ripemd160Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output);

// NEW: Vector-based PBKDF2 wrapper
bool pbkdf2HmacSha512Vector(const rust::Vec<uint8_t>& password, const rust::Vec<uint8_t>& salt, 
                            int count, size_t outputSize, rust::Vec<uint8_t>& output);

// Random wrappers
bool randomStatus();
void randomGetBytes(uint8_t* buffer, size_t size);
void randomAddEntropy(const uint8_t* buffer, size_t size, double entropy);

// NEW: Vector-based random function
void randomGetBytesVector(size_t size, rust::Vec<uint8_t>& output);

} // extern "C"
