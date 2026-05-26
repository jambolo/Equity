#include "crypto_wrapper.h"
#include <cstring>
#include <random>

namespace {

// Fills `buffer` with cryptographically-seeded random bytes using
// std::random_device. On every supported target std::random_device is
// OS-backed (BCryptGenRandom on Windows, /dev/urandom + getrandom on Linux).
void fill_random(uint8_t * buffer, size_t size) {
    static thread_local std::random_device rd;
    size_t i = 0;
    while (i + sizeof(uint32_t) <= size) {
        uint32_t v = rd();
        std::memcpy(buffer + i, &v, sizeof(v));
        i += sizeof(uint32_t);
    }
    if (i < size) {
        uint32_t v = rd();
        std::memcpy(buffer + i, &v, size - i);
    }
}

} // namespace

extern "C" {

bool randomStatus() {
    return true;
}

void randomGetBytes(uint8_t* buffer, size_t size) {
    fill_random(buffer, size);
}

void randomAddEntropy(const uint8_t* /*buffer*/, size_t /*size*/, double /*entropy*/) {
    // std::random_device does not accept user entropy; OS RNG handles its own seeding.
}

void randomGetBytesVector(size_t size, rust::Vec<uint8_t>& output) {
    output.clear();
    output.reserve(size);
    std::vector<uint8_t> temp(size);
    fill_random(temp.data(), size);
    for (uint8_t byte : temp) {
        output.push_back(byte);
    }
}

} // extern "C"
