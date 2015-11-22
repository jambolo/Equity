#include "Script.h"

#include "crypto/Ripemd.h"
#include "crypto/Sha1.h"
#include "crypto/Sha256.h"
#include "utility/Utility.h"
#include "utility/Serialize.h"

#include<algorithm>
#include <stack>

//OP_TRUE = OP_1,
//OP_FALSE = OP_0,

using namespace Equity;

struct OpcodeInfo
{
    char const * name;  // Name to display
    int nArgs;          // Minimum number of arguments
};
static OpcodeInfo const OPCODE_INFOS[] =
{
    { "0", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "1NEGATE", 0 },
    { "RESERVED", 0 }, { "1", 0 }, { "2", 0 }, { "3", 0 }, { "4", 0 }, { "5", 0 }, { "6", 0 }, { "7", 0 },
    { "8", 0 }, { "9", 0 }, { "10", 0 }, { "11", 0 }, { "12", 0 }, { "13", 0 }, { "14", 0 }, { "15", 0 },
    { "16", 0 }, { "NOP", 0 }, { "VER", 0 }, { "IF", 1 }, { "NOTIF", 1 }, { "VERIF", 0 }, { "VERNOTIF", 0 }, { "ELSE", 0 },
    { "ENDIF", 0 }, { "VERIFY", 1 }, { "RETURN", 0 }, { "TOALTSTACK", 1 }, { "FROMALTSTACK", 0 }, { "2DROP", 2 }, { "2DUP", 2 }, { "3DUP", 3 },
    { "2OVER", 4 }, { "2ROT", 6 }, { "2SWAP", 4 }, { "IFDUP", 1 }, { "DEPTH", 0 }, { "DROP", 1 }, { "DUP", 1 }, { "NIP", 2 },
    { "OVER", 2 }, { "PICK", 2 }, { "ROLL", 2 }, { "ROT", 3 }, { "SWAP", 2 }, { "TUCK", 2 }, { "CAT", 0 }, { "SUBSTR", 0 },
    { "LEFT", 0 }, { "RIGHT", 0 }, { "SIZE", 1 }, { "INVERT", 0 }, { "AND", 0 }, { "OR", 0 }, { "XOR", 0 }, { "EQUAL", 2 },
    { "EQUALVERIFY", 2 }, { "RESERVED1", 0 }, { "RESERVED2", 0 }, { "1ADD", 1 }, { "1SUB", 1 }, { "2MUL", 0 }, { "2DIV", 0 }, { "NEGATE", 1 },
    { "ABS", 1 }, { "NOT", 1 }, { "0NOTEQUAL", 1 }, { "ADD", 2 }, { "SUB", 2 }, { "MUL", 0 }, { "DIV", 0 }, { "MOD", 0 },
    { "LSHIFT", 0 }, { "RSHIFT", 0 }, { "BOOLAND", 2 }, { "BOOLOR", 2 }, { "NUMEQUAL", 2 }, { "NUMEQUALVERIFY", 2 }, { "NUMNOTEQUAL", 2 }, { "LESSTHAN", 2 },
    { "GREATERTHAN", 2 }, { "LESSTHANOREQUAL", 2 }, { "GREATERTHANOREQUAL", 2 }, { "MIN", 2 }, { "MAX", 2 }, { "WITHIN", 3 }, { "RIPEMD160", 1 }, { "SHA1", 1 },
    { "SHA256", 1 }, { "HASH160", 1 }, { "HASH256", 1 }, { "CODESEPARATOR", 0 }, { "CHECKSIG", 2 }, { "CHECKSIGVERIFY", 2 }, { "CHECKMULTISIG", 3 }, { "CHECKMULTISIGVERIFY", 3 },
    { "NOP1", 0 }, { "NOP2", 0 }, { "NOP3", 0 }, { "NOP4", 0 }, { "NOP5", 0 }, { "NOP6", 0 }, { "NOP7", 0 }, { "NOP8", 0 },
    { "NOP9", 0 }, { "NOP10", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 },
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 },
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 },
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 },
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 },
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 },
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 },
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 },
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "PUBKEYHASH", 0 }, { "PUBKEY", 0 }, { "INVALIDOPCODE", 0 },
};

Script::Instruction::Instruction(char const *& in, size_t & size)
{
    size_t instructionSize = 0;
    op_ = Utility::deserialize<uint8_t>(in, size);
    if (!in)
        return;
    instructionSize += 1;

    if (op_ >= 0x01 && op_ <= OP_PUSHDATA4)
    {
        size_t count;
        if (op_ >= 0 && op_ <= 0x4b)
        {
            count = op_;
        }
        else if (op_ == OP_PUSHDATA1)
        {
            count = Utility::deserialize<uint8_t>(in, size);
            if (!in)
                return;
            i += 1;
        }
        else if (op_ == OP_PUSHDATA2)
        {
            count = Utility::deserialize<uint16_t>(in, size);
            if (!in)
                return;
            i += 2;
        }
        else //if (op_ == OP_PUSHDATA4)
        {
            count = Utility::deserialize<uint32_t>(in, size);
            if (!in)
                return;
            i += 4;
        }

        data = Utility::deserializeBuffer(count, in, size);
        Instruction instruction = { op, std::vector<uint8_t>(data, data + count), location, 1 + count };
    }
    else if ((op_ == OP_VERIF || op_ == OP_VERNOTIF)
    {
        return false;
    }
    else if (op_ == OP_0 || (op_ >= OP_1NEGATE && op_ <= OP_NOP10))
    {
    }
    else
    {
        in = nullptr;
    }
}

Script::Script(std::vector<uint8_t> const & bytes)
{
    valid_ = parse(bytes);
}

void Script::serialize(std::vector<uint8_t> & out) const
{
    out.insert(out.end(), bytes_.begin(), bytes_.end());
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

bool Script::parse(std::vector<uint8_t> const & bytes)
{
    instructions_.clear();

    size_t i = 0;
    while (i < bytes.size())
    {
    }
    return true;
}

bool Script::check() const
{
    for (auto const & i : instructions_)
    {
        // Check for valid op codes (Note: all data push instructions have an op code of 0)
        if (!((i.op == 0) || (i.op >= OP_1NEGATE && i.op <= OP_NOP10)))
            return false;

        // Check for OP_VERIF or OP_VERNOTIF
        if (i.op == OP_VERIF || i.op == OP_VERNOTIF)
        {
            return false;
        }
        // Make sure that only data push instructions have data
        if ((i.op != 0) && i.data.size() > 0)
            return false;
    }

    return true;
}
