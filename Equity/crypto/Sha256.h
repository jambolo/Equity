#pragma once

#include <vector>
#include <cstdint>

class Sha256
{
public:
    std::vector<uint8_t> operator ()(std::vector<uint8_t> const & input);
};

