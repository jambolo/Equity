#pragma once

#include "equity/Instruction.h"
#include "p2p/Serialize.h"
#include <cstdint>
#include <stack>
#include <vector>

namespace Equity
{

//! A transaction script.
//!
//! A Script is processed by a node and if the result is not true, the transaction is rejected.
class Script : public P2p::Serializable
{
public:

    // Constructor
    //! @param      data    script in raw form
    Script(std::vector<uint8_t> const & data);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual json toJson() const override;

    //!@}

    //! Returns the script in a human-readable ("source-code") format
    std::string toSource() const;

    //! Returns true if the script is well-formed
    bool valid() const { return valid_; }

    //! Returns the script in raw form
    std::vector<uint8_t> data() const { return data_; }

    //! Returns the parsed script
    Program instructions() const { return instructions_; }

private:

    bool parse(std::vector<uint8_t> const & bytes);

    std::vector<uint8_t> data_;
    std::vector<Instruction> instructions_;
    bool valid_;
};

} // namespace Equity
