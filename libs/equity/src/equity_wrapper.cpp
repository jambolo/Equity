#include "equity_wrapper.h"

#include <cstring>
#include <stdexcept>

// Simple implementations for testing
uint32_t configurationGetNetworkId() {
    // Return a default network ID for testing
    return 0; // Mainnet
}

uint32_t configurationGetAddressVersion() {
    // Return default address version for mainnet
    return 0x00;
}

uint32_t configurationGetPrivateKeyVersion() {
    // Return default private key version for mainnet  
    return 0x80;
}

// Mnemonic functions - simplified for testing
bool mnemonicGenerate(uint32_t strength, rust::String& output) {
    // Simple test implementation - in real use would call Equity::Mnemonic::generate
    if (strength == 128) {
        output = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
        return true;
    }
    return false;
}

bool mnemonicValidate(rust::Str mnemonic) {
    // Simple validation - check if it's the test mnemonic
    std::string mnemonicStr(mnemonic);
    return mnemonicStr == "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
}

bool mnemonicToSeed(rust::Str mnemonic, rust::Str passphrase, rust::Vec<uint8_t>& seed) {
    // Simple test implementation - would normally call Equity::Mnemonic::toSeed
    std::string mnemonicStr(mnemonic);
    if (mnemonicStr == "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about") {
        // Return a test seed (64 bytes)
        seed.clear();
        for (int i = 0; i < 64; i++) {
            seed.push_back(static_cast<uint8_t>(i));
        }
        return true;
    }
    return false;
}
