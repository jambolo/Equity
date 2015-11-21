#include "Serialize.h"

#include "Utility.h"

#include <algorithm>

namespace Utility
{
    VarInt::VarInt(uint8_t const *& in, size_t & size)
    {
        if (size < 1)
        {
            in = nullptr;
            value_ = 0;
            return;
        }

        switch (*in)
        {
        case 0xff:  value_ = deserialize<uint64_t>(++in, --size); break;
        case 0xfe:  value_ = deserialize<uint32_t>(++in, --size); break;
        case 0xfd:  value_ = deserialize<uint16_t>(++in, --size); break;
        default:    value_ = *in++; --size; break;
        }
    }

    void VarInt::serialize(std::vector<uint8_t> & out) const
    {
        if (value_ < 0xfd)
        {
            out.push_back((uint8_t)value_);
        }
        else if (value_ < 0x10000)
        {
            out.reserve(out.size() + 3);
            out.push_back(0xfd);
            Utility::serialize((uint16_t)value_, out);
        }
        else if (value_ < 0x100000000)
        {
            out.reserve(out.size() + 5);
            out.push_back(0xfe);
            Utility::serialize((uint32_t)value_, out);
        }
        else
        {
            out.reserve(out.size() + 9);
            out.push_back(0xff);
            Utility::serialize((uint64_t)value_, out);
        }
    }

    template<>
    void serialize<uint8_t>(uint8_t const & a, std::vector<uint8_t> & out)
    {
        out.push_back(a);
    }
        
    template<>
    void serialize<uint16_t>(uint16_t const & a, std::vector<uint8_t> & out)
    {
        out.reserve(out.size() + 2);
        out.push_back( a       & 0xff);
        out.push_back((a >> 8) & 0xff);
    }
        
    template<>
    void serialize<uint32_t>(uint32_t const & a, std::vector<uint8_t> & out)
    {
        out.reserve(out.size() + 4);
        out.push_back( a        & 0xff);
        out.push_back((a >>  8) & 0xff);
        out.push_back((a >> 16) & 0xff);
        out.push_back((a >> 24) & 0xff);
    }
        
    template<>
    void serialize<uint64_t>(uint64_t const & a, std::vector<uint8_t> & out)
    {
        out.reserve(out.size() + 8);
        out.push_back( a        & 0xff);
        out.push_back((a >>  8) & 0xff);
        out.push_back((a >> 16) & 0xff);
        out.push_back((a >> 24) & 0xff);
        out.push_back((a >> 32) & 0xff);
        out.push_back((a >> 40) & 0xff);
        out.push_back((a >> 48) & 0xff);
        out.push_back((a >> 56) & 0xff);
    }
        
    template<>
    void serialize<std::vector<uint8_t> >(std::vector<uint8_t> const & a, std::vector<uint8_t> & out)
    {
        out.insert(out.end(), a.begin(), a.end());
    }
        
    template<>
    uint8_t deserialize<uint8_t>(uint8_t const *& in, size_t & size)
    {
        if (size < 1)
        {
            in = nullptr;
            return 0;
        }
        uint8_t out;
        out = in[0];
        in += 1;
        size -= 1;

        return out;
    }
        
    template<>
    uint16_t deserialize<uint16_t>(uint8_t const *& in, size_t & size)
    {
        if (size < 2)
        {
            in = nullptr;
            return 0;
        }
        uint16_t out =  (uint16_t)in[0] +
                       ((uint16_t)in[1] << 8);
        in += 2;
        size -= 2;
        return out;
    }
        
    template<>
    uint32_t deserialize<uint32_t>(uint8_t const *& in, size_t & size)
    {
        if (size < 4)
        {
            in = nullptr;
            return 0;
        }
        uint32_t out =  (uint32_t)in[0] +
                       ((uint32_t)in[1] <<  8) +
                       ((uint32_t)in[2] << 16) +
                       ((uint32_t)in[3] << 24);
        in += 4;
        size -= 4;
        return out;
    }
        
    template<>
    uint64_t deserialize<uint64_t>(uint8_t const *& in, size_t & size)
    {
        if (size < 8)
        {
            in = nullptr;
            return 0;
        }
        uint64_t out =  (uint64_t)in[0] +
                       ((uint64_t)in[1] <<  8) +
                       ((uint64_t)in[2] << 16) +
                       ((uint64_t)in[3] << 24) +
                       ((uint64_t)in[4] << 32) +
                       ((uint64_t)in[5] << 40) +
                       ((uint64_t)in[6] << 48) +
                       ((uint64_t)in[7] << 56);
        in += 8;
        size -= 8;
        return out;
    }
        
    std::vector<uint8_t> deserializeBuffer(size_t n, uint8_t const *& in, size_t & size)
    {
        if (size < n)
        {
            in = nullptr;
            return std::vector<uint8_t>();
        }
        std::vector<uint8_t> out;
        out.insert(out.end(), in, in + size);
        in += n;
        size -= n;
        return out;
    }

        
} // namespace Utility
