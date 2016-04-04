#include "Inventory.h"

#include "p2p/Serialize.h"
#include "utility/Endian.h"

using namespace Network;
using namespace Utility;

InventoryId::InventoryId(TypeId type, Crypto::Sha256Hash const & hash)
    : type_(type)
    , hash_(hash)
{
}

InventoryId::InventoryId(uint8_t const * & in, size_t & size)
{
    uint32_t type_u32 = littleEndian(P2p::deserialize<uint32_t>(in, size));
    if (type_u32 > TYPE_FILTERED_BLOCK)
        throw DeserializationError();
    type_ = static_cast<TypeId>(type_u32);
    hash_ = P2p::deserializeArray<Crypto::Sha256Hash>(in, size);
}

void InventoryId::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(littleEndian((uint32_t)type_), out);
    P2p::serializeArray(hash_, out);
}

