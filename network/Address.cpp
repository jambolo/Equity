#include "Address.h"

#include "p2p/Serialize.h"
#include "utility/Endian.h"

using namespace Network;
using namespace Utility;

Network::Address::Address()
{
}

Address::Address(uint32_t time, uint64_t services, std::array<uint8_t, 16> const & ipv6, uint16_t port)
    : time_(time)
    , services_(services)
    , ipv6_(ipv6)
    , port_(port)
{

}

Address::Address(uint8_t const * & in, size_t & size)
{
    time_ = littleEndian(P2p::deserialize<uint32_t>(in, size));
    services_ = littleEndian(P2p::deserialize<uint64_t>(in, size));
    ipv6_ = P2p::deserializeArray<std::array<uint8_t, 16> >(in, size);
    port_ = littleEndian(P2p::deserialize<uint16_t>(in, size));
}

void Address::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(littleEndian(time_), out);
    P2p::serialize(littleEndian(services_), out);
    P2p::serializeArray(ipv6_, out);
    P2p::serialize(littleEndian(port_), out);
}
