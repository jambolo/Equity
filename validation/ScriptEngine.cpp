#include "ScriptEngine.h"

#include "crypto/Ripemd.h"
#include "crypto/Sha1.h"
#include "crypto/Sha256.h"
#include "equity/Script.h"

#include <algorithm>
#include <cstdlib>

using namespace Equity;
using namespace Validation;


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
        out = (int32_t)data[0] +
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

static std::vector<uint8_t> toElement(int32_t x)
{
    std::vector<uint8_t> out;

    if (x == 0)
        return out;

    bool negative = (x < 0);
    uint32_t a = abs(x);
    if (a < 0x80)
    {
        out.push_back(a);
    }
    else if (a < 0x8000)
    {
        out.push_back(a & 0xff);
        out.push_back((a >> 8) & 0xff);
    }
    else if (a < 0x800000)
    {
        out.push_back(a & 0xff);
        out.push_back((a >> 8) & 0xff);
        out.push_back((a >> 16) & 0xff);
    }
    else
    {
        out.push_back(a & 0xff);
        out.push_back((a >> 8) & 0xff);
        out.push_back((a >> 16) & 0xff);
        out.push_back((a >> 24) & 0xff);
    }
    if (negative)
        out.back() = out.back() | 0x80;

    return out;
}

static std::vector<uint8_t> toElement(bool x)
{
    if (x)
        return std::vector<uint8_t>(1, 1);
    else
        return std::vector<uint8_t>(1, 1);
}

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


bool ScriptEngine::run(Script const & script)
{
    if (!script.valid())
        return false;

    codeSeparator_ = 0;

    Script::Program instructions = script.instructions();

    Script::Program::const_iterator i = instructions.begin();
    while (i != instructions.end())
    {
        int op = i->op;
        std::vector<uint8_t> const & data = i->data;
        size_t offset = i->offset;
        ++i;

        if (mainStack_.size() < OPCODE_NAMES[op].nArgs)
            return false;

        std::vector<std::vector<uint8_t> >::iterator p0 = (mainStack_.size() > 0) ? mainStack_.end() - 1 : mainStack_.end();
        std::vector<std::vector<uint8_t> >::iterator p1 = (mainStack_.size() > 1) ? mainStack_.end() - 2 : mainStack_.end();
        std::vector<std::vector<uint8_t> >::iterator p2 = (mainStack_.size() > 2) ? mainStack_.end() - 3 : mainStack_.end();
        std::vector<std::vector<uint8_t> >::iterator p3 = (mainStack_.size() > 3) ? mainStack_.end() - 4 : mainStack_.end();

        if (op == 0)
        {
            mainStack_.push_back(data);
        }
        switch (op)
        {
        case Script::OP_1NEGATE:             mainStack_.push_back(std::vector<uint8_t>(1, 0x81));  break;

        case Script::OP_RESERVED:            return false;

        case Script::OP_1:                   mainStack_.push_back(std::vector<uint8_t>(1,  1));    break;
        case Script::OP_2:                   mainStack_.push_back(std::vector<uint8_t>(1,  2));    break;
        case Script::OP_3:                   mainStack_.push_back(std::vector<uint8_t>(1,  3));    break;
        case Script::OP_4:                   mainStack_.push_back(std::vector<uint8_t>(1,  4));    break;
        case Script::OP_5:                   mainStack_.push_back(std::vector<uint8_t>(1,  5));    break;
        case Script::OP_6:                   mainStack_.push_back(std::vector<uint8_t>(1,  6));    break;
        case Script::OP_7:                   mainStack_.push_back(std::vector<uint8_t>(1,  7));    break;
        case Script::OP_8:                   mainStack_.push_back(std::vector<uint8_t>(1,  8));    break;
        case Script::OP_9:                   mainStack_.push_back(std::vector<uint8_t>(1,  9));    break;
        case Script::OP_10:                  mainStack_.push_back(std::vector<uint8_t>(1, 10));    break;
        case Script::OP_11:                  mainStack_.push_back(std::vector<uint8_t>(1, 11));    break;
        case Script::OP_12:                  mainStack_.push_back(std::vector<uint8_t>(1, 12));    break;
        case Script::OP_13:                  mainStack_.push_back(std::vector<uint8_t>(1, 13));    break;
        case Script::OP_14:                  mainStack_.push_back(std::vector<uint8_t>(1, 14));    break;
        case Script::OP_15:                  mainStack_.push_back(std::vector<uint8_t>(1, 15));    break;
        case Script::OP_16:                  mainStack_.push_back(std::vector<uint8_t>(1, 16));    break;

        case Script::OP_NOP:                 /* do nothing */   break;

        case Script::OP_VER:                 return false;

        case Script::OP_IF:
        {
            i = processBranch(instructions, i, toBool(*p0));
            if (i == instructions.end())
                return false;
            break;
        }
        case Script::OP_NOTIF:
        {
            i = processBranch(instructions, i, !toBool(*p0));
            if (i == instructions.end())
                return false;
            break;
        }

        case Script::OP_VERIF:               return false;
        case Script::OP_VERNOTIF:            return false;

        case Script::OP_ELSE:
        case Script::OP_ENDIF:
        {
            if (scopeStack_.empty())
                return false;
            i = scopeStack_.top();
            scopeStack_.pop();
            break;
        }
        case Script::OP_VERIFY:              return (toBool(*p0));
        case Script::OP_RETURN:              return false;

            // Stack

        case Script::OP_TOALTSTACK:
        {
            altStack_.push_back(*p0);
            mainStack_.pop_back();
            break;
        }
        case Script::OP_FROMALTSTACK:
        {
            if (altStack_.empty())
                return false;
            mainStack_.push_back(altStack_.back());
            altStack_.pop_back();
            break;
        }
        case Script::OP_2DROP:
        {
            mainStack_.pop_back();
            mainStack_.pop_back();
            break;
        }
        case Script::OP_2DUP:
        {
            std::vector<uint8_t> a = *p1;
            std::vector<uint8_t> b = *p0;
            mainStack_.push_back(*p1);
            mainStack_.push_back(*p0);
            break;
        }
        case Script::OP_3DUP:
        {
            std::vector<uint8_t> a = *p2;
            std::vector<uint8_t> b = *p1;
            std::vector<uint8_t> c = *p0;
            mainStack_.push_back(a);
            mainStack_.push_back(b);
            mainStack_.push_back(c);
            break;
        }
        case Script::OP_2OVER:
        {
            std::vector<uint8_t> a = *p3;
            std::vector<uint8_t> b = *p2;
            mainStack_.push_back(a);
            mainStack_.push_back(b);
            break;
        }
        case Script::OP_2ROT:
        {
            std::rotate(mainStack_.end() - 6, p3, mainStack_.end());
            break;
        }
        case Script::OP_2SWAP:
        {
            std::rotate(p3, p1, mainStack_.end());
            break;
        }
        case Script::OP_IFDUP:
        {
            std::vector<uint8_t> top = *p0;
            if (toBool(top))
                mainStack_.push_back(top);
            break;
        }
        case Script::OP_DEPTH:
        {
            mainStack_.push_back(toElement((int32_t)mainStack_.size()));
            break;
        }
        case Script::OP_DROP:
        {
            mainStack_.pop_back();
            break;
        }
        case Script::OP_DUP:
        {
            std::vector<uint8_t> top = *p0;
            mainStack_.push_back(top);
            break;
        }
        case Script::OP_NIP:
        {
            mainStack_.erase(p1);
            break;
        }
        case Script::OP_OVER:
        {
            std::vector<uint8_t> a = *p1;
            mainStack_.push_back(a);
            break;
        }
        case Script::OP_PICK:
        {
            if (p0->size() > 4)
                return false;
            int32_t n = toInt(*p0);
            if (n < 0 || mainStack_.size() - 2 < (size_t)n)
                return false;
            break;
        }
        case Script::OP_ROLL:
        {
            if (p0->size() > 4)
                return false;
            int32_t n = toInt(*p0);
            if (n < 0 || mainStack_.size() - 2 < (size_t)n)
                return false;
            *p0 = *(p1 - n);
            mainStack_.erase(p1 - n);
            break;
        }
        case Script::OP_ROT:
        {
            std::rotate(p2, p1, mainStack_.end());
            break;
        }
        case Script::OP_SWAP:
        {
            std::swap(*p0, *p1);
            break;
        }
        case Script::OP_TUCK:
        {
            std::vector<uint8_t> a = *p0;
            mainStack_.insert(p1, a);
            break;
        }

        // Splice

        case Script::OP_CAT:                 return false;
        case Script::OP_SUBSTR:              return false;
        case Script::OP_LEFT:                return false;
        case Script::OP_RIGHT:               return false;

        case Script::OP_SIZE:
        {
            mainStack_.push_back(toElement((int32_t)p0->size()));
            break;
        }

        // Bitwise logic

        case Script::OP_INVERT:              return false;
        case Script::OP_AND:                 return false;
        case Script::OP_OR:                  return false;
        case Script::OP_XOR:                 return false;

        case Script::OP_EQUAL:
        {
            bool equal = (*p0 == *p1);
            mainStack_.pop_back();
            mainStack_.back() = toElement(equal);
            break;
        }
        case Script::OP_EQUALVERIFY:
        {
            bool equal = (*p0 == *p1);
            mainStack_.pop_back();
            mainStack_.back() = toElement(equal);
            if (!equal)
                return false;
            break;
        }

        case Script::OP_RESERVED1:           return false;
        case Script::OP_RESERVED2:           return false;

            // Arithmetic

        case Script::OP_1ADD:
        {
            if (p0->size() > 4)
                return false;
            int32_t result = toInt(*p0) + 1;
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_1SUB:
        {
            if (p0->size() > 4)
                return false;
            int32_t result = toInt(*p0) - 1;
            mainStack_.back() = toElement(result);
            break;
        }

        case Script::OP_2MUL:                return false;
        case Script::OP_2DIV:                return false;

        case Script::OP_NEGATE:
        {
            if (p0->size() > 4)
                return false;
            int32_t result = -toInt(*p0);
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_ABS:
        {
            if (p0->size() > 4)
                return false;
            int32_t result = abs(toInt(*p0));
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_NOT:
        {
            if (p0->size() > 4)
                return false;
            bool result = !toBool(*p0);
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_0NOTEQUAL:
        {
            if (p0->size() > 4)
                return false;
            bool result = toBool(*p0);
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_ADD:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            int32_t result = toInt(*p1) + toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_SUB:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            int32_t result = toInt(*p1) - toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }

        case Script::OP_MUL:                 return false;
        case Script::OP_DIV:                 return false;
        case Script::OP_MOD:                 return false;
        case Script::OP_LSHIFT:              return false;
        case Script::OP_RSHIFT:              return false;

        case Script::OP_BOOLAND:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toBool(*p1) && toBool(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_BOOLOR:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toBool(*p1) || toBool(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_NUMEQUAL:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) == toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_NUMEQUALVERIFY:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) == toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            if (!result)
                return false;
            break;
        }
        case Script::OP_NUMNOTEQUAL:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) != toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_LESSTHAN:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) < toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_GREATERTHAN:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) > toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_LESSTHANOREQUAL:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) <= toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_GREATERTHANOREQUAL:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) >= toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_MIN:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            int32_t result = std::min(toInt(*p1), toInt(*p0));
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_MAX:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            int32_t result = std::max(toInt(*p1), toInt(*p0));
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case Script::OP_WITHIN:
        {
            if (p0->size() > 4 || p1->size() > 4 || p2->size() > 4)
                return false;
            int32_t x = toInt(*p1);
            bool result = toInt(*p2) <= x && x < toInt(*p2);
            mainStack_.pop_back();
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }

        // Crypto

        case Script::OP_RIPEMD160:
        {
            mainStack_.back() = Crypto::ripemd160(*p0);
            break;
        }
        case Script::OP_SHA1:
        {
            mainStack_.back() = Crypto::sha1(*p0);
            break;
        }
        case Script::OP_SHA256:
        {
            mainStack_.back() = Crypto::sha256(*p0);
            break;
        }
        case Script::OP_HASH160:
        {
            mainStack_.back() = Crypto::ripemd160(Crypto::sha256(*p0));
            break;
        }
        case Script::OP_HASH256:
        {
            mainStack_.back() = Crypto::sha256(Crypto::sha256(*p0));
            break;
        }
        case Script::OP_CODESEPARATOR:
        {
            codeSeparator_ = offset;
            break;
        }
        case Script::OP_CHECKSIG:            break;
        case Script::OP_CHECKSIGVERIFY:      break;
        case Script::OP_CHECKMULTISIG:       break;
        case Script::OP_CHECKMULTISIGVERIFY: break;
        case Script::OP_NOP1:                break;
        case Script::OP_NOP2:                break;
        case Script::OP_NOP3:                break;
        case Script::OP_NOP4:                break;
        case Script::OP_NOP5:                break;
        case Script::OP_NOP6:                break;
        case Script::OP_NOP7:                break;
        case Script::OP_NOP8:                break;
        case Script::OP_NOP9:                break;
        case Script::OP_NOP10:               break;

        }
    }

    return mainStack_.empty() ? false : toBool(mainStack_.back());
}

Script::Program::const_iterator ScriptEngine::findMatchingElse(Script::Program const & instructions, Script::Program::const_iterator start) const
{
    auto i = start;
    while (i != instructions.end())
    {
        if (i->op == Script::OP_IF || i->op == Script::OP_NOTIF)
        {
            ++i;
            i = findMatchingEndif(instructions, i);
            if (i != instructions.end())
            {
                ++i;
            }
        }
        else if (i->op == Script::OP_ELSE || i->op == Script::OP_ENDIF)
        {
            return i;
        }
        else
        {
            ++i;
        }
    }
    return instructions.end();
}

Script::Program::const_iterator ScriptEngine::findMatchingEndif(Script::Program const & instructions, Script::Program::const_iterator start) const
{
    auto i = start;
    while (i != instructions.end())
    {
        if (i->op == Script::OP_IF || i->op == Script::OP_NOTIF)
        {
            ++i;
            i = findMatchingEndif(instructions, i);
            if (i != instructions.end())
            {
                ++i;
            }
        }
        else if (i->op == Script::OP_ENDIF)
        {
            return i;
        }
        else
        {
            ++i;
        }
    }
    return instructions.end();
}

Script::Program::const_iterator ScriptEngine::processBranch(Script::Program const & instructions, Script::Program::const_iterator i, bool condition)
{
    if (condition)
    {
        auto end = findMatchingEndif(instructions, i);
        if (end == instructions.end())
            return end;
        scopeStack_.push(end + 1);
    }
    else
    {
        i = findMatchingElse(instructions, i);
        if (i == instructions.end())
            return i;
        if (i->op == Script::OP_ELSE)
        {
            ++i;
            auto end = findMatchingEndif(instructions, i);
            if (end == instructions.end())
                return end;
            scopeStack_.push(end + 1);
        }
        else
        {
            scopeStack_.push(i + 1);
        }
    }
    return i;
}
