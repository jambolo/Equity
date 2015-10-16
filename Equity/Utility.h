#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <crypto/Sha256.h>

namespace Equity
{
    namespace Utility
    {
        class BlockHeader
        {
        public:
            BlockHeader(uint8_t const *& out, size_t & size);

            void serialize(std::vector<uint8_t> & out);

        private:

            int32_t version;
            Crypto::Sha256Hash previousBlock;
            Crypto::Sha256Hash merkleRoot;
            uint32_t timestamp;
            uint32_t target;
            uint32_t nonce;
            uint64_t count;
        };

        //! Converts a vector to a hex string
        std::string vtox(std::vector<uint8_t> const & v);

        //! Converts a vector to a hex string
        std::string vtox(uint8_t const * v, size_t length);

        //! Converts a hex string to a vector
        std::vector<uint8_t> xtov(std::string const & x);

        //! Converts a hex string to a vector
        std::vector<uint8_t> xtov(char const * x, size_t length);

        //! Computes the Merkle Tree root of a list of SHA-256 hashes
        Crypto::Sha256Hash merkleRoot(std::vector<Crypto::Sha256Hash> hashes);

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


        template<typename T>
        T deserialize(uint8_t const *& in, size_t & size)
        {
            return T(in, size);
        }

        template<>
        std::string deserialize<std::string>(uint8_t const *& in, size_t & size);

        template<>
        uint64_t deserialize<uint64_t>(uint8_t const *& in, size_t & size);

    } // namespace Utility
} // namespace Equity