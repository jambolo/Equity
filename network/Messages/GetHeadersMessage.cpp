#include "GetHeadersMessage.h"

#include "p2p/Serialize.h"
#include "utility/Endian.h"

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
    version_ = littleEndian(P2p::deserialize<uint32_t>(in, size));
    hashes_ = P2p::VarArray<Crypto::Sha256Hash>(in, size).value();
    last_ = P2p::deserializeArray<Crypto::Sha256Hash>(in, size);
}

void GetHeadersMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(littleEndian(version_), out);
    P2p::serialize(P2p::VarArray<Crypto::Sha256Hash>(hashes_), out);
    P2p::serialize(last_, out);
}

json GetHeadersMessage::toJson() const
{
    return std::string("...");
}
