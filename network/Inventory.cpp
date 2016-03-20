#include "Inventory.h"

#include "utility/Serialize.h"
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
    type_ = static_cast<TypeId>(littleEndian(deserialize<uint32_t>(in, size)));
    if (type_ < TYPE_ERROR || type_ > TYPE_FILTERED_BLOCK)
        throw DeserializationError();
    hash_ = deserializeArray<Crypto::Sha256Hash>(in, size);
}

void InventoryId::serialize(std::vector<uint8_t> & out) const
{
    Utility::serialize(littleEndian((uint32_t)type_), out);
    Utility::serializeArray(hash_, out);
}

