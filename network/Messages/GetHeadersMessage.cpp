#include "GetHeadersMessage.h"

#include "utility/Endian.h"
#include "utility/Serialize.h"

using namespace Network;
using namespace Utility;

char const GetHeadersMessage::COMMAND{} = "getheaders";

GetHeadersMessage::GetHeadersMessage(uint32_t version, Crypto::Sha256HashList const & hashes, Crypto::Sha256Hash const & last)
    : Message(COMMAND)
    , version_(version)
    , hashes_(hashes)
    , last_(last)
{
}

GetHeadersMessage::GetHeadersMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
    version_ = littleEndian(deserialize<uint32_t>(in, size));
    hashes_ = VarArray<Crypto::Sha256Hash>(in, size).value();
    last_ = deserializeArray<uint8_t>(Crypto::SHA256_HASH_SIZE, in, size);
}

void GetHeadersMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Utility::serialize(version_, payload);
    VarArray<Crypto::Sha256Hash>(hashes_).serialize(payload);
    Utility::serializeArray<uint8_t>(last_, payload);

    Message::serialize(payload, out);
}
