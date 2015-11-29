#include "Message.h"

#include "crypto/Sha256.h"
#include "network/Configuration.h"
#include "utility/Endian.h"
#include "utility/Serialize.h"

#include <algorithm>
#include <cassert>

using namespace Network;

Message::Message(char const * command)
    : magic_(Network::Configuration::get().network)
    , command_(command)
    , checksum_(0)
{
    assert(magic_ == MAGIC_MAIN || magic_ == MAGIC_TEST || magic_ == MAGIC_TEST3);
    assert(!command_.empty() && command_.length() < COMMAND_SIZE);
}

void Message::serialize(std::vector<uint8_t> const & payload, std::vector<uint8_t> & out) const
{
    // Network ID (4 uint32_t, little-endian)
    Utility::serialize(Utility::littleEndian(magic_), out);

    // Command (11 bytes + 0 terminator, padded with 0s)
    assert(command_.length() < COMMAND_SIZE);
    out.insert(out.end(), COMMAND_SIZE, 0);
    std::copy(command_.begin(), command_.end(), out.end() - COMMAND_SIZE);

    // Payload size (uint32_t, little-endian)
    Utility::serialize(Utility::littleEndian((uint32_t)payload.size()), out);

    // Checksum (4 bytes)
    std::vector<uint8_t> check = Crypto::checksum(payload);
    Utility::serializeArray(check, out);

    // Payload
    Utility::serializeArray(payload, out);
}
