#include "Configuration.h"

#include "p2p/Message.h"
#include <cstdint>

namespace
{

Network::Configuration instance_ =
{
    P2p::Message::Header::MAGIC_MAIN,
    8333
};

}
