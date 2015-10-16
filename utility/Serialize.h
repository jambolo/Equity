#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <crypto/Sha256.h>

namespace Utility
{
    
    class VarInt
    {
    public:
        VarInt(uint64_t v) : value_(v) {}
        void serialize(std::vector<uint8_t> & out);
    private:
        uint64_t value_;
    };
    
    template<typename T>
    class VarArray
    {
    public:
        VarArray(std::vector<T> const & v) : vector_(v) {}
        void serialize(std::vector<uint8_t> & out)
        {
            VarInt(vector_.size()).serialize(out);
            for (typename std::vector<T>::const_iterator i = vector_.begin(); i != vector_.end(); ++i)
            {
                i->serialize(out);
            }
        }
    private:
        std::vector<T> const & vector_;
    };
    
    template<>
    class VarArray<uint8_t>
    {
    public:
        VarArray(std::vector<uint8_t> const & v) : vector_(v) {}
        void serialize(std::vector<uint8_t> & out)
        {
            VarInt(vector_.size()).serialize(out);
            out.insert(out.end(), vector_.begin(), vector_.end());
        }
    private:
        std::vector<uint8_t> const & vector_;
    };
    
    
    template<typename T>
    void serialize(T const & a, std::vector<uint8_t> & out)
    {
        return a.serialize(out);
    }
    
    template<> void serialize<uint8_t>(uint8_t const & a, std::vector<uint8_t> & out);
    template<> void serialize<uint16_t>(uint16_t const & a, std::vector<uint8_t> & out);
    template<> void serialize<uint32_t>(uint32_t const & a, std::vector<uint8_t> & out);
    template<> void serialize<uint64_t>(uint64_t const & a, std::vector<uint8_t> & out);
    template<> void serialize<std::vector<uint8_t> >(std::vector<uint8_t> const & a, std::vector<uint8_t> & out);
    
    template<typename T>
    T deserialize(uint8_t const *& in, size_t & size)
    {
        return T(in, size);
    }
    
    template<> uint8_t  deserialize<uint8_t >(uint8_t const *& in, size_t & size);
    template<> uint16_t deserialize<uint16_t>(uint8_t const *& in, size_t & size);
    template<> uint32_t deserialize<uint32_t>(uint8_t const *& in, size_t & size);
    template<> uint64_t deserialize<uint64_t>(uint8_t const *& in, size_t & size);
    std::vector<uint8_t>  deserializeBuffer(size_t n, uint8_t const *& in, size_t & size);
    
} // namespace Utility
