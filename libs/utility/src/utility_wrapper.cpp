#include "utility_wrapper.h"
#include "Endian.h"
#include "Utility.h"
#include <cstring>
#include <stdexcept>

extern "C" {

// Endian conversion functions
uint16_t utilityEndianSwap16(uint16_t x) {
    return Utility::Endian::swap(x);
}

uint32_t utilityEndianSwap32(uint32_t x) {
    return Utility::Endian::swap(x);
}

uint64_t utilityEndianSwap64(uint64_t x) {
    return Utility::Endian::swap(x);
}

uint16_t utilityEndianLittle16(uint16_t x) {
    return Utility::Endian::little(x);
}

uint32_t utilityEndianLittle32(uint32_t x) {
    return Utility::Endian::little(x);
}

uint64_t utilityEndianLittle64(uint64_t x) {
    return Utility::Endian::little(x);
}

uint16_t utilityEndianBig16(uint16_t x) {
    return Utility::Endian::big(x);
}

uint32_t utilityEndianBig32(uint32_t x) {
    return Utility::Endian::big(x);
}

uint64_t utilityEndianBig64(uint64_t x) {
    return Utility::Endian::big(x);
}

// Utility hex conversion functions
bool utilityToHex(const rust::Vec<uint8_t>& input, rust::String& output) {
    try {
        std::vector<uint8_t> data(input.begin(), input.end());
        std::string result = Utility::toHex(data);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityToHexPtr(const uint8_t* data, size_t length, rust::String& output) {
    try {
        if (!data && length > 0) {
            return false;
        }
        std::string result = Utility::toHex(data, length);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityFromHex(const rust::String& hex, rust::Vec<uint8_t>& output) {
    try {
        std::string hex_str(hex);
        std::vector<uint8_t> result = Utility::fromHex(hex_str);
        
        output.clear();
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityFromHexPtr(const int8_t* hex, size_t length, rust::Vec<uint8_t>& output) {
    try {
        if (!hex && length > 0) {
            return false;
        }
        std::vector<uint8_t> result = Utility::fromHex(reinterpret_cast<const char*>(hex), length);
        
        output.clear();
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityToHexR(const rust::Vec<uint8_t>& input, rust::String& output) {
    try {
        std::vector<uint8_t> data(input.begin(), input.end());
        std::string result = Utility::toHexR(data);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityToHexRPtr(const uint8_t* data, size_t length, rust::String& output) {
    try {
        if (!data && length > 0) {
            return false;
        }
        std::string result = Utility::toHexR(data, length);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityFromHexR(const rust::String& hex, rust::Vec<uint8_t>& output) {
    try {
        std::string hex_str(hex);
        std::vector<uint8_t> result = Utility::fromHexR(hex_str);
        
        output.clear();
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityFromHexRPtr(const int8_t* hex, size_t length, rust::Vec<uint8_t>& output) {
    try {
        if (!hex && length > 0) {
            return false;
        }
        std::vector<uint8_t> result = Utility::fromHexR(reinterpret_cast<const char*>(hex), length);
        
        output.clear();
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityShorten(const rust::String& input, size_t size, rust::String& output) {
    try {
        std::string input_str(input);
        std::string result = Utility::shorten(input_str, size);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}


} // extern "C"
