#include "Utility.h"

#include <algorithm>

static char itox(int i)
{
    return (i < 10) ? '0' + i : 'a' + (i - 10);
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

    std::string toHex(std::vector<uint8_t> const & v)
{
    if (v.empty())
    {
        return toHex(NULL, 0);
    }
    else
    {
        return toHex(&v[0], v.size());
    }
}

std::string toHex(uint8_t const * v, size_t length)
{
    if (length == 0)
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

    return fromHex(&x[0], x.size());
}

std::vector<uint8_t> fromHex(char const * x, size_t length)
{
    if (length == 0)
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

/********************************************************************************************************************/
/*                                          J S O N   C O N V E R S I O N                                           */
/********************************************************************************************************************/

static std::string const DOUBLE_QUOTE("\"");
static std::string const EMPTY_JSON_STRING("\"\"");

template <>
std::string toJson<uint8_t>(std::vector<uint8_t> const & v)
{
    if (v.empty())
    {
        return EMPTY_JSON_STRING;
    }
    else
    {
        return DOUBLE_QUOTE + toHex(v) + DOUBLE_QUOTE;
    }
}

std::vector<uint8_t> fromJson(std::string const & j)
{
    if (j.length() < 3 || j.front() != '\"' || j.back() != '\"')
    {
        return std::vector<uint8_t>();
    }
    else
    {
        return fromHex(j.data() + 1, j.length() - 2);
    }
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
