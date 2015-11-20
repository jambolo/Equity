#include "Utility.h"

#include <algorithm>

static char itox(int i)
{
    return (i < 10) ? '0' + i : 'a' + (i - 10);
}
static int xtoi(unsigned x)
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
std::string vtox(std::vector<uint8_t> const & v)
{
    if (v.empty())
    {
        return vtox(NULL, 0);
    }
    else
    {
        return vtox(&v[0], v.size());
    }
}
std::string vtox(uint8_t const * v, size_t length)
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
std::vector<uint8_t> xtov(std::string const & x)
{
    if (x.empty())
    {
        return std::vector<uint8_t>();
    }

    return xtov(&x[0], x.size());
}
std::vector<uint8_t> xtov(char const * x, size_t length)
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
Crypto::Sha256Hash merkleRoot(std::vector<Crypto::Sha256Hash> hashes)
{
    std::vector<Crypto::Sha256Hash> last = hashes;
    if (last.size() == 1)
    {
        last.push_back(last.back());
    }

    while (last.size() > 1)
    {
        std::vector<Crypto::Sha256Hash> next;
        if ((last.size() & 1) == 1)
        {
            last.push_back(last.back());
        }
        for (int i = 0; i < last.size(); i += 2)
        {
            std::vector<uint8_t> two;
            two.insert(two.end(), last[i].begin(), last[i].end());
            two.insert(two.end(), last[i + 1].begin(), last[i + 1].end());
            next.push_back(Crypto::sha256(two));
        }
        last.swap(next);
    }

    return last[0];
}
static std::string const DOUBLE_QUOTE("\"");
static std::string const EMPTY_JSON_STRING("\"\"");

std::string toJson(std::vector<uint8_t> const & v)
{
    if (v.empty())
    {
        return EMPTY_JSON_STRING;
    }
    else
    {
        return DOUBLE_QUOTE + vtox(v) + DOUBLE_QUOTE;
    }
}
std::vector<uint8_t> jtov(std::string const & j)
{
    if (j.length() < 3 || j.front() != '\"' || j.back() != '\"')
    {
        return std::vector<uint8_t>();
    }
    else
    {
        return xtov(j.data() + 1, j.length() - 2);
    }
}
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
