#include "Script.h"

#include "crypto/Ripemd.h"
#include "crypto/Sha1.h"
#include "crypto/Sha256.h"
#include "utility/Utility.h"

#include<algorithm>
#include <stack>

//OP_TRUE = OP_1,
//OP_FALSE = OP_0,

using namespace Equity;

struct OpcodeInfo
{
    char const * name;
    int nArgs;
};
static OpcodeInfo const OPCODE_NAMES[] =
{
    { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 }, { "PUSHDATA", 0 },
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
        size_t offset = i;
        int op = bytes[i];
        ++i;

        if (op >= OP_0 && op <= OP_PUSHDATA4)
        {
            uint8_t const * size = &bytes[i];
            size_t count;
            if (op >= 0 && op <= 0x4b)
            {
                count = op;
            }
            else if (op == OP_PUSHDATA1)
            {
                if (i + 1 > bytes.size())
                    return false;
                count = (size_t)size[0];
                i += 1;
            }
            else if (op == OP_PUSHDATA2)
            {
                if (i + 2 > bytes.size())
                    return false;
                count = (size_t)size[0] + ((size_t)size[1] << 8);
                i += 2;
            }
            else //if (op == OP_PUSHDATA4)
            {
                if (i + 4 > bytes.size())
                    return false;
                count = (size_t)size[0] + ((size_t)size[1] << 8) + ((size_t)size[2] << 16) + ((size_t)size[3] << 24);
                i += 4;
            }

            // Note: All data push instructions have an op of 0 because the details of how the data is stored are irrelevant
            if (i + count > bytes.size())
                return false;
            uint8_t const * data = &bytes[i];
            Instruction instruction = { 0, std::vector<uint8_t>(data, data + count), offset };
            instructions_.push_back(instruction);
            i += count;
        }
        else if (op == OP_VERIF || OP_VERNOTIF)
        {
            return false;
        }
        else if (op >= OP_1NEGATE && op <= OP_NOP10)
        {
            Instruction instruction = { op, std::vector<uint8_t>(), offset };
            instructions_.push_back(instruction);
        }
        // not sure if these should be counted as valid
        //          else if (op >= OP_PUBKEYHASH && op <= OP_INVALIDOPCODE)
        //          {
        //              instructions_.push_back({ op, std::vector<uint8_t>() });
        //          }
        else
        {
            return false;
        }
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
