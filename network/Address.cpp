#include "Address.h"

#include "utility/Serialize.h"
#include "utility/Endian.h"

using namespace Network;
using namespace Utility;

Address::Address(uint32_t time, uint64_t services, std::vector<uint8_t> const & ipv6, uint16_t port)
    : time_(time)
    , services_(services)
    , ipv6_(ipv6)
    , port_(port)
{

}

Address::Address(uint8_t const *& in, size_t & size)
{
    time_ = littleEndian(deserialize<uint32_t>(in, size));
    services_ = littleEndian(deserialize<uint64_t>(in, size));
    ipv6_ = deserializeArray<uint8_t>(16, in, size);
    port_ = littleEndian(deserialize<uint16_t>(in, size));
}

void Address::serialize(std::vector<uint8_t> & out) const
{
    Utility::serialize(littleEndian(time_), out);
    Utility::serialize(littleEndian(services_), out);
    Utility::serializeArray<uint8_t>(ipv6_, out);
    Utility::serialize(littleEndian(port_), out);
}
