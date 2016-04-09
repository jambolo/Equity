#include "VersionMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const VersionMessage::TYPE[] = "version";

VersionMessage::VersionMessage()
    : Message(TYPE)
{
}

VersionMessage::VersionMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    version_ = P2p::deserialize<uint32_t>(in, size);
    services_ = P2p::deserialize<uint64_t>(in, size);
    timestamp_ = P2p::deserialize<uint64_t>(in, size);
    to_ = P2p::deserialize<Address>(in, size);

    // Fields below require version >= 106
    if (version_ >= 106)
    {
        from_ = P2p::deserialize<Address>(in, size);
        uint64_t nonce_ = P2p::deserialize<uint32_t>(in, size);

        userAgent_ = P2p::VarString(in, size);
        height_ = P2p::deserialize<uint32_t>(in, size);

        // Fields below require version >= 70001
        if (version_ >= 70001)
            relay_ = P2p::deserialize<uint8_t>(in, size) != 0;
    }
}

void VersionMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(version_, out);
    P2p::serialize(services_, out);
    P2p::serialize(timestamp_, out);
    P2p::serialize(to_, out);
    P2p::serialize(from_, out);
    P2p::serialize(nonce_, out);
    P2p::serialize(userAgent_, out);
    P2p::serialize(height_, out);
    P2p::serialize(static_cast<uint8_t>(relay_), out);
}
