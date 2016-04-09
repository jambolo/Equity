#pragma once

#include "equity/Instruction.h"
#include "p2p/Serialize.h"
#include <cstdint>
#include <stack>
#include <vector>

namespace Equity
{

class Script : public P2p::Serializable
{
public:

    Script(std::vector<uint8_t> const & data);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    std::string toJson() const;
    std::string toHex() const;
    std::string toSource() const;


    bool valid() const { return valid_; }
    std::vector<uint8_t> data() const { return data_; }
    Program instructions() const { return instructions_; }

private:

    bool parse(std::vector<uint8_t> const & bytes);

    std::vector<uint8_t> data_;
    std::vector<Instruction> instructions_;
    bool valid_;
};

} // namespace Equity
