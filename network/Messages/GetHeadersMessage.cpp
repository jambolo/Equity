#include "GetHeadersMessage.h"

#include "utility/Endian.h"
#include "utility/Serialize.h"

using namespace Network;
using namespace Utility;

char const GetHeadersMessage::TYPE[] = "getheaders";

GetHeadersMessage::GetHeadersMessage(uint32_t version, Crypto::Sha256HashList const & hashes, Crypto::Sha256Hash const & last)
    : Message(TYPE)
    , version_(version)
    , hashes_(hashes)
    , last_(last)
{
}

GetHeadersMessage::GetHeadersMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    version_ = littleEndian(deserialize<uint32_t>(in, size));
    hashes_ = VarArray<Crypto::Sha256Hash>(in, size).value();
    last_ = deserializeArray<Crypto::Sha256Hash>(in, size);
}

void GetHeadersMessage::serialize(std::vector<uint8_t> & out) const
{
    Utility::serialize(littleEndian(version_), out);
    VarArray<Crypto::Sha256Hash>(hashes_).serialize(out);
    Utility::serializeArray<Crypto::Sha256Hash>(last_, out);
}
