#include "Script.h"

#include "utility/Utility.h"
#include <stack>

//OP_TRUE = OP_1,
//OP_FALSE = OP_0,

using namespace Equity;

static bool toBool(std::vector<uint8_t> const & data)
{
    // An empty vector represents FALSE
    if (data.empty())
        return false;

    // Anything not equal to [00 00 ...] or [80 00 ...] is TRUE
    if (data.front() != 0 && data.front() != 0x80)
        return true;
    for (auto x : data)
    {
        if (x != 0)
            return true;
    }

    // Must be [00 00 ...] or [80 00 ...], so FALSE
    return false;
}

static int32_t toInt(std::vector<uint8_t> const & data)
{
    if (data.empty())
        return 0;

    int32_t out;
    bool negative = ((data.back() & 0x80) != 0);

    if (data.size() >= 4)
    {
        out =  (int32_t)data[0] +
              ((int32_t)data[1] << 8) +
              ((int32_t)data[2] << 16) +
              ((int32_t)(data[3] & 0x7f) << 24);
    }
    else if (data.size() == 3)
    {
        out = (int32_t)data[0] +
             ((int32_t)data[1] << 8) +
             ((int32_t)(data[2] & 0x7f) << 16);
    }
    else if (data.size() == 2)
    {
        out = (int32_t)data[0] +
             ((int32_t)(data[1] & 0x7f) << 8);
    }
    else
    {
        out = (int32_t)(data[0] & 0x7f);
    }

    if (negative)
    {
        out = -out;
    }
    return out;
}

static char const * const OPCODE_NAMES[] =
{
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA",
    "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "PUSHDATA", "1NEGATE",
    "RESERVED", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", "10", "11", "12", "13", "14", "15",
    "16", "NOP", "VER", "IF", "NOTIF", "VERIF", "VERNOTIF", "ELSE",
    "ENDIF", "VERIFY", "RETURN", "TOALTSTACK", "FROMALTSTACK", "2DROP", "2DUP", "3DUP",
    "2OVER", "2ROT", "2SWAP", "IFDUP", "DEPTH", "DROP", "DUP", "NIP",
    "OVER", "PICK", "ROLL", "ROT", "SWAP", "TUCK", "CAT", "SUBSTR",
    "LEFT", "RIGHT", "SIZE", "INVERT", "AND", "OR", "XOR", "EQUAL",
    "EQUALVERIFY", "RESERVED1", "RESERVED2", "1ADD", "1SUB", "2MUL", "2DIV", "NEGATE",
    "ABS", "NOT", "0NOTEQUAL", "ADD", "SUB", "MUL", "DIV", "MOD",
    "LSHIFT", "RSHIFT", "BOOLAND", "BOOLOR", "NUMEQUAL", "NUMEQUALVERIFY", "NUMNOTEQUAL", "LESSTHAN",
    "GREATERTHAN", "LESSTHANOREQUAL", "GREATERTHANOREQUAL", "MIN", "MAX", "WITHIN", "RIPEMD160", "SHA1",
    "SHA256", "HASH160", "HASH256", "CODESEPARATOR", "CHECKSIG", "CHECKSIGVERIFY", "CHECKMULTISIG", "CHECKMULTISIGVERIFY",
    "NOP1", "NOP2", "NOP3", "NOP4", "NOP5", "NOP6", "NOP7", "NOP8",
    "NOP9", "NOP10", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)",
    "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)",
    "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)",
    "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)",
    "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)",
    "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)",
    "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)",
    "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)",
    "(invalid)", "(invalid)", "(invalid)", "(invalid)", "(invalid)", "PUBKEYHASH", "PUBKEY", "INVALIDOPCODE"
};

Script::Script(std::vector<uint8_t> const & bytes)
{
    valid_ = parse(bytes);
}

Script::Script(std::vector<Instruction> const & instructions)
    : instructions_(instructions)
{
    valid_ = check();
}

void Script::serialize(std::vector<uint8_t> & out)
{

}

bool Script::run() const
{
    if (!valid_)
        return false;

    std::stack<std::vector<uint8_t> > mainStack;
    std::stack<std::vector<uint8_t> > altStack;

    struct Branch
    {
        std::vector<Instruction>::const_iterator end;
        std::vector<Instruction>::const_iterator resume;
    };
    std::stack<Branch> branchStack;

    auto i = instructions_.begin();
    while (i != instructions_.end())
    {
        int op = i->op;
        std::vector<uint8_t> const & data = i->data;
        ++i;

        if (op == 0)
        {
            mainStack.push(data);
        }
        switch (op)
        {
        case OP_1NEGATE:             mainStack.push({ 0x81 }); break;
        case OP_RESERVED:            return false;
        case OP_1:                   mainStack.push({  1 });  break;
        case OP_2:                   mainStack.push({  2 });  break;
        case OP_3:                   mainStack.push({  3 });  break;
        case OP_4:                   mainStack.push({  4 });  break;
        case OP_5:                   mainStack.push({  5 });  break;
        case OP_6:                   mainStack.push({  6 });  break;
        case OP_7:                   mainStack.push({  7 });  break;
        case OP_8:                   mainStack.push({  8 });  break;
        case OP_9:                   mainStack.push({  9 });  break;
        case OP_10:                  mainStack.push({ 10 });  break;
        case OP_11:                  mainStack.push({ 11 });  break;
        case OP_12:                  mainStack.push({ 12 });  break;
        case OP_13:                  mainStack.push({ 13 });  break;
        case OP_14:                  mainStack.push({ 14 });  break;
        case OP_15:                  mainStack.push({ 15 });  break;
        case OP_16:                  mainStack.push({ 16 });  break;
        case OP_NOP:                 /* do nothing */break;
        case OP_VER:                 return false;
        case OP_IF:
        {
            if (toBool(data))
            {
                auto end = findMatchingElseOrEndif(i);
                if (end == instructions_.end())
                    return false;

                auto resume = findMatchingEndif(i);
                if (resume == instructions_.end())
                    return false;
                branchStack.push({ end, resume });
            }
            else
            {
                i = findMatchingElseOrEndif(i);
                if (i != instructions_.end())
                {
                    auto end = findMatchingEndif();
                    branchStack.push({ end, end });
                }
                else
                {

                }
            }
            break;
        }
        case OP_NOTIF:               break;
        case OP_VERIF:               return false;
        case OP_VERNOTIF:            return false;
        case OP_ELSE:                break;
        case OP_ENDIF:               break;
        case OP_VERIFY:              break;
        case OP_RETURN:              break;

            // Stack

        case OP_TOALTSTACK:          break;
        case OP_FROMALTSTACK:        break;
        case OP_2DROP:               break;
        case OP_2DUP:                break;
        case OP_3DUP:                break;
        case OP_2OVER:               break;
        case OP_2ROT:                break;
        case OP_2SWAP:               break;
        case OP_IFDUP:               break;
        case OP_DEPTH:               break;
        case OP_DROP:                break;
        case OP_DUP:                 break;
        case OP_NIP:                 break;
        case OP_OVER:                break;
        case OP_PICK:                break;
        case OP_ROLL:                break;
        case OP_ROT:                 break;
        case OP_SWAP:                break;
        case OP_TUCK:                break;

            // Splice

        case OP_CAT:                 break;
        case OP_SUBSTR:              break;
        case OP_LEFT:                break;
        case OP_RIGHT:               break;
        case OP_SIZE:                break;

            // Bitwise logic

        case OP_INVERT:              break;
        case OP_AND:                 break;
        case OP_OR:                  break;
        case OP_XOR:                 break;
        case OP_EQUAL:               break;
        case OP_EQUALVERIFY:         break;
        case OP_RESERVED1:           break;
        case OP_RESERVED2:           break;

            // Arithmetic

        case OP_1ADD:                break;
        case OP_1SUB:                break;
        case OP_2MUL:                break;
        case OP_2DIV:                break;
        case OP_NEGATE:              break;
        case OP_ABS:                 break;
        case OP_NOT:                 break;
        case OP_0NOTEQUAL:           break;
        case OP_ADD:                 break;
        case OP_SUB:                 break;
        case OP_MUL:                 break;
        case OP_DIV:                 break;
        case OP_MOD:                 break;
        case OP_LSHIFT:              break;
        case OP_RSHIFT:              break;
        case OP_BOOLAND:             break;
        case OP_BOOLOR:              break;
        case OP_NUMEQUAL:            break;
        case OP_NUMEQUALVERIFY:      break;
        case OP_NUMNOTEQUAL:         break;
        case OP_LESSTHAN:            break;
        case OP_GREATERTHAN:         break;
        case OP_LESSTHANOREQUAL:     break;
        case OP_GREATERTHANOREQUAL:  break;
        case OP_MIN:                 break;
        case OP_MAX:                 break;
        case OP_WITHIN:              break;

            // Crypto

        case OP_RIPEMD160:           break;
        case OP_SHA1:                break;
        case OP_SHA256:              break;
        case OP_HASH160:             break;
        case OP_HASH256:             break;
        case OP_CODESEPARATOR:       break;
        case OP_CHECKSIG:            break;
        case OP_CHECKSIGVERIFY:      break;
        case OP_CHECKMULTISIG:       break;
        case OP_CHECKMULTISIGVERIFY: break;
        case OP_NOP1:                break;
        case OP_NOP2:                break;
        case OP_NOP3:                break;
        case OP_NOP4:                break;
        case OP_NOP5:                break;
        case OP_NOP6:                break;
        case OP_NOP7:                break;
        case OP_NOP8:                break;
        case OP_NOP9:                break;
        case OP_NOP10:               break;

        }
    }
}

std::string Script::toJson()
{

}

std::string Script::toHex()
{
    std::vector<uint8_t> out;
    serialize(out);
    return Utility::vtox(out);
}

bool Script::parse(std::vector<uint8_t> const & bytes)
{
    instructions_.clear();

    size_t i = 0;
    while (i < bytes.size())
    {
        int op = bytes[i++];

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
            else if (op == OP_PUSHDATA4)
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
            instructions_.push_back({ 0, std::vector<uint8_t>(data, data + count) });
            i += count;
        }
        else if (op >= OP_1NEGATE && op <= OP_NOP10)
        {
            instructions_.push_back({ op, std::vector<uint8_t>() });
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
};

bool Script::check() const
{
    for (auto i : instructions_)
    {
        // Check for valid op codes (Note: all data push instructions have an op code of 0)
        if (!((i.op == 0) || (i.op >= OP_1NEGATE && i.op <= OP_NOP10)))
            return false;

        // Make sure that only data push instructions have data
        if ((i.op != 0) && i.data.size() > 0)
            return false;
    }
}

std::vector<Script::Instruction>::const_iterator Script::findMatchingElseOrEndif(std::vector<Script::Instruction>::const_iterator start) const
{
    auto i = start;
    while ( i != instructions_.end())
    {
        if (i->op == OP_IF || i->op == OP_NOTIF)
        {
            ++i;
            i = findMatchingEndif(i);
            if (i != instructions_.end())
            {
                ++i;
            }
        }
        else if (i->op == OP_ELSE || i->op == OP_ENDIF)
        {
            return i;
        }
        else
        {
            ++i;
        }
    }
    return instructions_.end();
}

std::vector<Script::Instruction>::const_iterator Script::findMatchingEndif(std::vector<Script::Instruction>::const_iterator start) const
{
    auto i = start;
    while (i != instructions_.end())
    {
        if (i->op == OP_IF || i->op == OP_NOTIF)
        {
            ++i;
            i = findMatchingEndif(i);
            if (i != instructions_.end())
            {
                ++i;
            }
        }
        else if (i->op == OP_ENDIF)
        {
            return i;
        }
        else
        {
            ++i;
        }
    }
    return instructions_.end();
}
