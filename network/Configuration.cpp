#include "Configuration.h"

#include "p2p/Message.h"
#include <cstdint>

Network::Configuration Network::Configuration::instance_ =
{
    P2p::Message::Header::MAGIC_MAIN,
    8333
};
