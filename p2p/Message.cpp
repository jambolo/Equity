#include "Message.h"

#include "crypto/Sha256.h"
#include "p2p/Serialize.h"
#include "utility/Endian.h"

#include <cassert>

using namespace Utility;
using namespace P2p;

Message::Message(char const * type, std::vector<uint8_t> const & payload)
    : type_(type)
    , payload_(payload)
{
    assert(!type_.empty() && type_.length() < Header::TYPE_SIZE);
}

void Message::Header::serialize(std::vector<uint8_t> & out) const
{
    // Network ID (uint32_t, little-endian)
    P2p::serialize(Endian::little(magic_), out);

    // Type (up to 11 bytes + 0 terminator, padded with 0s)
    out.insert(out.end(), type_, type_ + TYPE_SIZE);

    // Payload size (uint32_t, little-endian)
    P2p::serialize(Endian::little((uint32_t)length_), out);

    // Checksum (uint32_t, little-endian)
    // FIXME: Little-endian?
    P2p::serialize(Endian::little((uint32_t)checksum_), out);
}
