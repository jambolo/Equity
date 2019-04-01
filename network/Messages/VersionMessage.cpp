#include "VersionMessage.h"

#include "p2p/Serialize.h"

using json = nlohmann::json;
using namespace Network;

char const VersionMessage::TYPE[] = "version";

VersionMessage::VersionMessage(uint32_t            version,
                               uint64_t            services,
                               uint64_t            timestamp,
                               Address const &     to,
                               Address const &     from,
                               uint64_t            nonce,
                               std::string const & userAgent,
                               uint32_t            height,
                               bool                relay)
    : Message(TYPE)
    , version_(version)
    , services_(services)
    , timestamp_(timestamp)
    , to_(to)
    , from_(from)
    , nonce_(nonce)
    , userAgent_(userAgent)
    , height_(height)
    , relay_(relay)
{}

VersionMessage::VersionMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    version_   = P2p::deserialize<uint32_t>(in, size);
    services_  = P2p::deserialize<uint64_t>(in, size);
    timestamp_ = P2p::deserialize<uint64_t>(in, size);
    to_        = P2p::deserialize<Address>(in, size);

    // Fields below require version >= 106
    if (version_ >= 106)
    {
        from_      = P2p::deserialize<Address>(in, size);
        nonce_     = P2p::deserialize<uint32_t>(in, size);
        userAgent_ = P2p::VarString(in, size).value();
        height_    = P2p::deserialize<uint32_t>(in, size);

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

json VersionMessage::toJson() const
{
    json j =
    {
        { "version",   version_     },
        { "services",  services_    },
        { "timestamp", timestamp_   },
        { "to",        to_.toJson() }
    };

    // Fields below require version >= 106
    if (version_ >= 106)
    {
        j["from"]      = from_.toJson();
        j["nonce"]     = nonce_;
        j["userAgent"] = userAgent_;
        j["height"]    = height_;

        // Fields below require version >= 70001
        if (version_ >= 70001)
            j["relay"] = relay_;
    }

    return j;
}
