#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Network
{

    class Message
    {
    public:

        static uint32_t const   MAGIC_MAIN = 0xD9B4BEF9;
        static uint32_t const   MAGIC_TEST = 0xDAB5BFFA;
        static uint32_t const   MAGIC_TEST3 = 0x0709110B;

        Message(uint32_t m, std::string const & c);
        Message(uint8_t const *& in, size_t & size);

        virtual void serialize(std::vector<uint8_t> & out) const = 0;

    protected:

        //! Called by the derived class to serialize the entire message after building the payload
        void serialize(std::vector<uint8_t> const & payload, std::vector<uint8_t> & out) const;

    private:

        uint32_t magic_;
        std::string command_;
    };

}