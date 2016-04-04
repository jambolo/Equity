#include "Message.h"

#include "crypto/Sha256.h"
#include "network/Configuration.h"
#include "p2p/Message.h"
#include "utility/Endian.h"

#include <cassert>

using namespace Network;
using namespace Utility;

Message::Message(char const * type)
    : type_(type)
{
    assert(!type_.empty() && type_.length() < P2p::Message::Header::TYPE_SIZE);
}
