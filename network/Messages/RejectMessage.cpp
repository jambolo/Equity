#include "RejectMessage.h"

#include "p2p/Serialize.h"
#include "utility/Endian.h"

using namespace Network;

char const RejectMessage::TYPE[] = "reject";

RejectMessage::RejectMessage(std::string const & message, Code code, std::string const & reason, std::vector<uint8_t> const & data)
    : Message(TYPE)
    , message_(message)
    , code_(code)
    , reason_(reason)
    , data_(data)
{
}

RejectMessage::RejectMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    message_ = P2p::VarString(in, size).value();
    code_ = P2p::deserialize<uint8_t>(in, size);
    reason_ = P2p::VarString(in, size).value();
    data_ = P2p::deserializeVector<uint8_t>(size, in, size);
}

void RejectMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(P2p::VarString(message_), out);
    P2p::serialize(Utility::Endian::little(code_), out);
    P2p::serialize(P2p::VarString(reason_), out);
    P2p::serialize(data_, out);
}
