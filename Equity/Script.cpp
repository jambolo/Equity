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

std::string Script::toJson() const
{
    return std::string();
}

std::string Script::toHex() const
{
    std::vector<uint8_t> out;
    serialize(out);
    return Utility::toHex(out);
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
