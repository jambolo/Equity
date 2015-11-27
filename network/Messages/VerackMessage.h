#pragma once

#include "network/Message.h"

namespace Network
{

    class VerackMessage : public Message
    {
    public:

        VerackMessage(uint32_t m, std::string const & c);
        VerackMessage(uint8_t const * & in, size_t & size);

        virtual void serialize(std::vector<uint8_t> & out) const;

    private:

    };

} // namespace Network
