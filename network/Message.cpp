#include "Message.h"

#include "crypto/Sha256.h"
#include "utility/Serialize.h"

using namespace Network;

Message::Message(uint32_t m, std::string const & c)
    : magic_(m)
    , command_(c)
{
}

Message::Message(uint8_t const * & in, size_t & size)
    : magic_(0)
{
    magic_ = Utility::deserialize<uint32_t>(in, size);
    std::vector<uint8_t> commandBuffer = Utility::deserializeArray<uint8_t>(12, in, size);
    commandBuffer.push_back(0); // Ensure termination
    command_ = (char *)commandBuffer.data();
}

void Message::serialize(std::vector<uint8_t> const & payload, std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> commandBuffer(12);
    command_.copy((char *)commandBuffer.data(), command_.length());
    Crypto::Sha256Hash check = Crypto::sha256(payload);
    check.resize(4);

    Utility::serialize(magic_, out);
    Utility::serializeArray(commandBuffer, out);
    Utility::serialize((uint32_t)payload.size(), out);
    Utility::serializeArray(check, out);
    Utility::serializeArray(payload, out);
}
