#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{

namespace Base58
{

std::string encode(std::vector<uint8_t> const & input);
std::string encode(uint8_t const * input, size_t length);
bool decode(std::string const & input, std::vector<uint8_t> & output);
bool decode(char const * input, std::vector<uint8_t> & output);

}

}
