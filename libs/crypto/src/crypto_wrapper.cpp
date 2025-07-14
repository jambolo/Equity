#include "crypto_wrapper.h"
#include "crypto/Ecc.h"
#include "crypto/Hmac.h"
#include "crypto/Pbkdf2.h"
#include "crypto/Random.h"
#include "crypto/Ripemd.h"
#include "crypto/Sha1.h"
#include "crypto/Sha256.h"
#include "crypto/Sha512.h"
#include <cstring>

// Wrapper function implementations for cxx bridge compatibility

extern "C" {

// SHA-1 wrapper
void sha1(const uint8_t* input, size_t length, uint8_t* output) {
    auto result = Crypto::sha1(input, length);
    std::memcpy(output, result.data(), result.size());
}

// SHA-256 wrappers
void sha256(const uint8_t* input, size_t length, uint8_t* output) {
    auto result = Crypto::sha256(input, length);
    std::memcpy(output, result.data(), result.size());
}

void doubleSha256(const uint8_t* input, size_t length, uint8_t* output) {
    auto result = Crypto::doubleSha256(input, length);
    std::memcpy(output, result.data(), result.size());
}

void checksum(const uint8_t* input, size_t length, uint8_t* output) {
    auto result = Crypto::checksum(input, length);
    std::memcpy(output, result.data(), result.size());
}

// SHA-512 wrapper
void sha512(const uint8_t* input, size_t length, uint8_t* output) {
    auto result = Crypto::sha512(input, length);
    std::memcpy(output, result.data(), result.size());
}

// RIPEMD-160 wrapper
void ripemd160(const uint8_t* input, size_t length, uint8_t* output) {
    auto result = Crypto::ripemd160(input, length);
    std::memcpy(output, result.data(), result.size());
}

// HMAC-SHA512 wrapper
void hmacSha512(const uint8_t* key, size_t keySize, const uint8_t* message, size_t messageSize, uint8_t* output) {
    auto result = Crypto::hmacSha512(key, keySize, message, messageSize);
    std::memcpy(output, result.data(), result.size());
}

// PBKDF2 wrapper - returns success/failure and fills output buffer
bool pbkdf2HmacSha512(const uint8_t* password, size_t passwordSize, 
                      const uint8_t* salt, size_t saltSize, 
                      int count, size_t outputSize, uint8_t* output) {
    try {
        auto result = Crypto::pbkdf2HmacSha512(password, passwordSize, salt, saltSize, count, outputSize);
        if (result.size() != outputSize) {
            return false;
        }
        std::memcpy(output, result.data(), outputSize);
        return true;
    } catch (...) {
        return false;
    }
}

// ECC wrappers
bool eccPublicKeyIsValid(const uint8_t* k, size_t size) {
    return Crypto::Ecc::publicKeyIsValid(k, size);
}

bool eccPrivateKeyIsValid(const uint8_t* k, size_t size) {
    return Crypto::Ecc::privateKeyIsValid(k, size);
}

// NEW: ECC key derivation wrapper
bool eccDerivePublicKey(const uint8_t* privateKey, rust::Vec<uint8_t>& publicKey, bool uncompressed) {
    try {
        Crypto::Ecc::PrivateKey privKey;
        std::memcpy(privKey.data(), privateKey, Crypto::Ecc::PRIVATE_KEY_SIZE);
        
        Crypto::Ecc::PublicKey pubKey;
        bool result = Crypto::Ecc::derivePublicKey(privKey, pubKey, uncompressed);
        
        if (result) {
            publicKey.clear();
            publicKey.reserve(pubKey.size());
            for (uint8_t byte : pubKey) {
                publicKey.push_back(byte);
            }
        }
        
        return result;
    } catch (...) {
        return false;
    }
}

// NEW: ECC signing wrapper  
bool eccSign(const uint8_t* message, size_t messageSize, const uint8_t* privateKey, rust::Vec<uint8_t>& signature) {
    try {
        Crypto::Ecc::PrivateKey privKey;
        std::memcpy(privKey.data(), privateKey, Crypto::Ecc::PRIVATE_KEY_SIZE);
        
        Crypto::Ecc::Signature sig;
        bool result = Crypto::Ecc::sign(message, messageSize, privKey, sig);
        
        if (result) {
            signature.clear();
            signature.reserve(sig.size());
            for (uint8_t byte : sig) {
                signature.push_back(byte);
            }
        }
        
        return result;
    } catch (...) {
        return false;
    }
}

// NEW: ECC verification wrapper
bool eccVerify(const uint8_t* message, size_t messageSize, const rust::Vec<uint8_t>& publicKey, const rust::Vec<uint8_t>& signature) {
    try {
        Crypto::Ecc::PublicKey pubKey(publicKey.begin(), publicKey.end());
        Crypto::Ecc::Signature sig(signature.begin(), signature.end());
        
        return Crypto::Ecc::verify(message, messageSize, pubKey, sig);
    } catch (...) {
        return false;
    }
}

// NEW: Vector-based hash functions for convenience
void sha1Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output) {
    std::vector<uint8_t> inputVec(input.begin(), input.end());
    auto result = Crypto::sha1(inputVec);
    
    output.clear();
    output.reserve(result.size());
    for (uint8_t byte : result) {
        output.push_back(byte);
    }
}

void sha256Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output) {
    std::vector<uint8_t> inputVec(input.begin(), input.end());
    auto result = Crypto::sha256(inputVec);
    
    output.clear();
    output.reserve(result.size());
    for (uint8_t byte : result) {
        output.push_back(byte);
    }
}

void doubleSha256Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output) {
    std::vector<uint8_t> inputVec(input.begin(), input.end());
    auto result = Crypto::doubleSha256(inputVec);
    
    output.clear();
    output.reserve(result.size());
    for (uint8_t byte : result) {
        output.push_back(byte);
    }
}

void checksumVector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output) {
    std::vector<uint8_t> inputVec(input.begin(), input.end());
    auto result = Crypto::checksum(inputVec);
    
    output.clear();
    output.reserve(result.size());
    for (uint8_t byte : result) {
        output.push_back(byte);
    }
}

void sha512Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output) {
    std::vector<uint8_t> inputVec(input.begin(), input.end());
    auto result = Crypto::sha512(inputVec);
    
    output.clear();
    output.reserve(result.size());
    for (uint8_t byte : result) {
        output.push_back(byte);
    }
}

void ripemd160Vector(const rust::Vec<uint8_t>& input, rust::Vec<uint8_t>& output) {
    std::vector<uint8_t> inputVec(input.begin(), input.end());
    auto result = Crypto::ripemd160(inputVec);
    
    output.clear();
    output.reserve(result.size());
    for (uint8_t byte : result) {
        output.push_back(byte);
    }
}

// NEW: Vector-based PBKDF2 wrapper
bool pbkdf2HmacSha512Vector(const rust::Vec<uint8_t>& password, const rust::Vec<uint8_t>& salt, 
                            int count, size_t outputSize, rust::Vec<uint8_t>& output) {
    try {
        std::vector<uint8_t> passwordVec(password.begin(), password.end());
        std::vector<uint8_t> saltVec(salt.begin(), salt.end());
        
        auto result = Crypto::pbkdf2HmacSha512(passwordVec, saltVec, count, outputSize);
        
        output.clear();
        output.reserve(result.size());
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

// Random wrappers
bool randomStatus() {
    return Crypto::Random::status();
}

void randomGetBytes(uint8_t* buffer, size_t size) {
    Crypto::Random::getBytes(buffer, size);
}

void randomAddEntropy(const uint8_t* buffer, size_t size, double entropy) {
    Crypto::Random::addEntropy(buffer, size, entropy);
}

// NEW: Vector-based random function
void randomGetBytesVector(size_t size, rust::Vec<uint8_t>& output) {
    output.clear();
    // Allocate a temporary buffer since rust::Vec doesn't have resize
    std::vector<uint8_t> temp(size);
    Crypto::Random::getBytes(temp.data(), size);
    
    // Copy to rust::Vec
    for (uint8_t byte : temp) {
        output.push_back(byte);
    }
}

} // extern "C"
