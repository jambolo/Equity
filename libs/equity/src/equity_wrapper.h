#pragma once

#include "rust/cxx.h"
#include <cstdint>

// CXX-compatible function declarations for Equity bridge
// These use cxx's Rust types which are compatible with the bridge

// Configuration functions
uint32_t configurationGetNetworkId();
uint32_t configurationGetAddressVersion();
uint32_t configurationGetPrivateKeyVersion();

// Mnemonic functions
bool mnemonicGenerate(uint32_t strength, rust::String& output);
bool mnemonicValidate(rust::Str mnemonic);
bool mnemonicToSeed(rust::Str mnemonic, rust::Str passphrase, rust::Vec<uint8_t>& seed);
