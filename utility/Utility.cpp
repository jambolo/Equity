#include "Utility.h"

#include "p2p/Serialize.h"

#include <algorithm>

static char itox(int i)
{
    static char const MAP[] = "0123456789abcdef";
    return MAP[i];
}

static int xtoi(char x)
{
    if (x >= 'a')
    {
        return x - 'a' + 10;
    }
    else if (x >= 'A')
    {
        return x - 'A' + 10;
    }
    else
    {
        return x - '0';
}
}

namespace Utility
{

/********************************************************************************************************************/
/*                                           H E X   C O N V E R S I O N                                            */
/********************************************************************************************************************/

std::string toHex(P2p::Serializable const & s)
{
    std::vector<uint8_t> out;
    s.serialize(out);
    return toHex(out);
}

std::string toHex(std::vector<uint8_t> const & v)
{
    if (v.empty())
    {
        return toHex(nullptr, 0);
    }
    else
    {
        return toHex(v.data(), v.size());
    }
}

std::string toHex(uint8_t const * v, size_t length)
{
    if (!v || length == 0)
    {
        return std::string();
    }

    std::string x;
    x.reserve(length * 2);

    for (int i = 0; i < length; ++i)
    {
        x.push_back(itox(*v >> 4));
        x.push_back(itox(*v & 0x0f));
        ++v;
    }

    return x;
}

std::vector<uint8_t> fromHex(std::string const & x)
{
    if (x.empty())
    {
        return std::vector<uint8_t>();
    }

    return fromHex(x.data(), x.size());
}

std::vector<uint8_t> fromHex(char const * x, size_t length)
{
    if (!x || length == 0)
    {
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> v;
    v.reserve(length / 2);

    for (int i = 0; i < length; i += 2)
    {
        v.push_back((xtoi(x[i + 0]) << 4) + xtoi(x[i + 1]));
    }

    return v;
}

std::string toHexR(std::vector<uint8_t> const & v)
{
    if (v.empty())
    {
        return std::string();
    }
    else
    {
        return toHexR(v.data(), v.size());
    }
}

std::string toHexR(uint8_t const * v, size_t length)
{
    if (!v || length == 0)
    {
        return std::string();
    }

    std::string x;
    x.reserve(length * 2);

    v += length;
    for (int i = 0; i < length; ++i)
    {
        --v;
        x.push_back(itox((*v >> 4) & 0x0f));
        x.push_back(itox(*v & 0x0f));
    }

    return x;
}

std::vector<uint8_t> fromHexR(std::string const & x)
{
    if (x.empty())
    {
        return std::vector<uint8_t>();
    }

    return fromHexR(x.data(), x.size());
}

std::vector<uint8_t> fromHexR(char const * x, size_t length)
{
    if (!x || length == 0)
    {
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> v;
    v.reserve(length / 2);

    for (size_t i = length; i > 0; i -= 2)
    {
        v.push_back((xtoi(x[i - 2]) << 4) + xtoi(x[i - 1]));
    }

    return v;
}

/********************************************************************************************************************/
/*                                            M I S C E L L A N E O U S                                             */
/********************************************************************************************************************/

std::string shorten(std::string const & in, size_t size /* = 11*/)
{
    size_t const ELLIPSIS_SIZE = 3;

    if (size >= in.length())
    {
        return in;
    }
    else if (size > ELLIPSIS_SIZE)
    {
        size_t prefixSize = (size - ELLIPSIS_SIZE + 1) / 2;
        size_t suffixSize = (size - ELLIPSIS_SIZE) / 2;

        return in.substr(0, prefixSize) + "..." + in.substr(in.length() - suffixSize, suffixSize);
    }
    else
    {
        return in.substr(0, size);
    }
}

}     // namespace Utility
