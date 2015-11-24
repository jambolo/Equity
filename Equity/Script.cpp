#include "Script.h"

#include "crypto/Ripemd.h"
#include "crypto/Sha1.h"
#include "crypto/Sha256.h"
#include "utility/Serialize.h"
#include "utility/Utility.h"

#include <algorithm>
#include <stack>

using namespace Equity;

Script::Script(std::vector<uint8_t> const & data)
    : data_(data)
{
    valid_ = parse(data);
}

void Script::serialize(std::vector<uint8_t> & out) const
{
    out.insert(out.end(), data_.begin(), data_.end());
}

static std::string DOUBLE_QUOTE("\"");

static std::string toString(Instruction const i)
{
    
    if (i.op() >= 0x01 && i.op() <= Instruction::OP_1NEGATE)
    {
        return Utility::toHex(i.data());
    }
    else
    {
        return i.description().name;
    }
}

std::string Script::toJson() const
{
    std::string out = DOUBLE_QUOTE + Utility::toHex(data_) + DOUBLE_QUOTE;

    return out;
}

std::string Script::toHex() const
{
    return Utility::toHex(data_);
}

std::string Script::toSource() const
{
    std::string out;
    if (!instructions_.empty())
    {
        Program::const_iterator i = instructions_.begin();
        out += toString(*i++);
        while (i != instructions_.end())
        {
            out += ' ';
            out += toString(*i++);
        }
    }
    return out;
}

bool Script::parse(std::vector<uint8_t> const & raw)
{
    instructions_.clear();

    uint8_t const * p = raw.data();
    size_t size = raw.size();
    while (p != nullptr && size > 0)
    {
        try
        {
            Instruction i(p, size, p - raw.data());
            instructions_.push_back(i);
        }
        catch (Utility::DeserializationError)
        {
            return false;
        }
    }

    return true;
}
