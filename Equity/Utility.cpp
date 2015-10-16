#include "Utility.h"

#include <algorithm>

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

namespace Equity
{
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

        Crypto::Sha256Hash merkleRoot(std::vector<Crypto::Sha256Hash> hashes)
        {
            std::vector<Crypto::Sha256Hash> last = hashes;
            if (last.size() == 1)
                last.push_back(last.back());

            while (last.size() > 1)
            {
                std::vector<Crypto::Sha256Hash> next;
                if (last.size() & 1 == 1)
                    last.push_back(last.back());
                for (int i = 0; i < last.size(); i += 2)
                {
                    std::vector<uint8_t> two;
                    two.insert(two.end(), last[i].begin(), last[i].end());
                    two.insert(two.end(), last[i+1].begin(), last[i+1].end());
                    next.push_back(Crypto::sha256(two));
                }
                last.swap(next);
            }

            return last[0];
        }

        void VarInt::serialize(std::vector<uint8_t> & out)
        {
            if (value_ < 0xfd)
            {
                out.push_back((uint8_t)value_);
            }
            else if (value_ < 0x10000)
            {
                out.reserve(out.size() + 3);
                out.push_back(0xfd);
                serialize((uint16_t)value_, out);
            }
            else if (value_ < 0x100000000)
            {
                out.reserve(out.size() + 5);
                out.push_back(0xfe);
                serialize((uint32_t)value_, out);
            }
            else
            {
                out.reserve(out.size() + 9);
                out.push_back(0xff);
                serialize((uint64_t)value_, out);
            }
        }

        bool serialize(std::string const & s, uint8_t *& out, size_t & size)
        {
            size_t length = s.size();
            if (!serialize((uint64_t)length, out, size))
                return false;
            if (size < length)
                return false;
            s.copy((char *)out, length);
            out += length;
            size -= length;

            return true;
        }

        bool serialize(char const * s, uint8_t *& out, size_t & size)
        {
            return serialize(s, strlen(s), out, size);
        }

        template<>
        std::string deserialize<std::string>(uint8_t const *& in, size_t & size)
        {
            size_t length = (size_t)deserialize<uint64_t>(in, size);

            if (in == NULL || size < length)
            {
                in = NULL;
                return std::string();
            }

            std::string out(in, in + length);
            in += length;
            size -= length;

            return out;
        }

        template<>
        uint64_t deserialize<uint64_t>(uint8_t const *& in, size_t & size)
        {
            if (size == 0)
            {
                in = NULL;
                return 0;
            }

            uint64_t out = 0;

            if (in[0] == 0xff)
            {
                if (size >= 9)
                {
                    out = in[8];
                    out = (out << 8) + in[7];
                    out = (out << 8) + in[6];
                    out = (out << 8) + in[5];
                    out = (out << 8) + in[4];
                    out = (out << 8) + in[3];
                    out = (out << 8) + in[2];
                    out = (out << 8) + in[1];
                    in += 9;
                    size -= 9;
                }
                else
                {
                    in = NULL;
                }
            }
            else if (in[0] == 0xfe)
            {
                if (size >= 5)
                {
                    out = in[4];
                    out = (out << 8) + in[3];
                    out = (out << 8) + in[2];
                    out = (out << 8) + in[1];
                    in += 5;
                    size -= 5;
                }
                else
                {
                    in = NULL;
                }
            }
            else if (in[0] == 0xfd)
            {
                if (size >= 3)
                {
                    out = in[2];
                    out = (out << 8) + in[1];
                    in += 3;
                    size -= 3;
                }
                else
                {
                    in = NULL;
                }
            }
            else
            {
                out = in[0];
            }
            return out;
        }


    } // namespace Utility
} // namespace Equity