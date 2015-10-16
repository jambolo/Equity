#include "Message.h"

#include "equity/Utility.h"

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

    Equity::Utility::serialize(magic_, out);
    Equity::Utility::serialize(commandBuffer, out);
    Equity::Utility::serialize((uint32_t)payload.size(), out);
    Equity::Utility::serialize(check, out);
    Equity::Utility::serialize(payload, out);
}
