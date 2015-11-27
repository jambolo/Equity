#include "Configuration.h"

#include "network/Message.h"
#include <cstdint>

namespace Network
{

    Configuration instance_ =
    {
        Message::MAGIC_MAIN,
        8333
    };

}