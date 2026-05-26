#include "VerackMessage.h"

#include "p2p/Serialize.h"
#include <nlohmann/json.hpp>

using namespace Network;

char const VerackMessage::TYPE[] = "verack";

VerackMessage::VerackMessage()
    : Message(TYPE)
{
}

VerackMessage::VerackMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void VerackMessage::serialize(std::vector<uint8_t> & out) const
{
}

nlohmann::json VerackMessage::toJson() const
{
    return nlohmann::json::object();
}
