#pragma once

#include <cstdint>
#include <cstddef>
#include "rust/cxx.h"

// C wrapper function declarations for cxx bridge compatibility

extern "C" {

// Random wrappers
bool randomStatus();
void randomGetBytes(uint8_t* buffer, size_t size);
void randomAddEntropy(const uint8_t* buffer, size_t size, double entropy);

// NEW: Vector-based random function
void randomGetBytesVector(size_t size, rust::Vec<uint8_t>& output);

} // extern "C"
