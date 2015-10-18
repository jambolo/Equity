#include "Message.h"

#include "utility/Serialize.h"

using namespace Network;


Message::Message(uint32_t m, std::string const & c)
    : magic_(m)
    , command_(c)
{
}

void Message::serialize(std::vector<uint8_t> const & payload, std::vector<uint8_t> & out)
{
    std::vector<uint8_t>commandBuffer(12);
    command_.copy((char *)&commandBuffer[0], command_.length());
    Crypto::Sha256Hash check = Crypto::sha256(payload);
    check.resize(4);

    Utility::serialize(magic_, out);
    Utility::serialize(commandBuffer, out);
    Utility::serialize((uint32_t)payload.size(), out);
    Utility::serialize(check, out);
    Utility::serialize(payload, out);
}
