#include "Utility.h"

static char itox(int i)
{
    return (i < 10) ? '0' + i : 'A' + (i - 10);
}

static int xtoi(unsigned x)
{
    if (x >= 'a')
        return x - 'a' + 10;
    else if (x >= 'A')
        return x - 'A' + 10;
    else
        return x - '0';
}

namespace Utility
{

    std::string vtox(std::vector<uint8_t> const & v)
    {
        if (v.empty())
            return std::string();

        return vtox(&v[0], v.size());
    }

    std::string vtox(uint8_t const * v, size_t length)
    {
        std::string x;
        x.reserve(length * 2);

        for (int i = 0; i < length; ++i)
        {
            x.push_back(*v >> 4);
            x.push_back(*v & 0x0f);
            ++v;
        }

        return x;
    }

    std::vector<uint8_t> xtov(std::string const & x)
    {
        if (x.empty())
            return std::vector<uint8_t>();

        return xtov(&x[0], x.size());
    }

    std::vector<uint8_t> xtov(char const * x, size_t length)
    {
        std::vector<uint8_t> v;
        v.reserve(length / 2);

        for (int i = 0; i < length; i += 2)
        {
            v.push_back((xtoi(x[i + 0]) << 4) + x[i + 1]);
        }

        return v;
    }

} // namespace Utility
