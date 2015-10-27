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
    { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "(invalid)", 0 }, { "PUBKEYHASH", 0 }, { "PUBKEY", 0 }, { "INVALIDOPCODE"               0 },
};

Script::Script(std::vector<uint8_t> const & bytes)
{
    valid_ = parse(bytes);
}

Script::Script(Program const & instructions)
    : instructions_(instructions)
{
    valid_ = check();
}

void Script::serialize(std::vector<uint8_t> & out) const
{

}

bool Script::run()
{
    if (!valid_)
        return false;

    auto i = instructions_.begin();
    while (i != instructions_.end())
    {
        int op = i->op;
        std::vector<uint8_t> const & data = i->data;
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
        case OP_1NEGATE:             mainStack_.push_back({ 0x81 });  break;

        case OP_RESERVED:            return false;

        case OP_1:                   mainStack_.push_back({ 1 });    break;
        case OP_2:                   mainStack_.push_back({ 2 });    break;
        case OP_3:                   mainStack_.push_back({ 3 });    break;
        case OP_4:                   mainStack_.push_back({ 4 });    break;
        case OP_5:                   mainStack_.push_back({ 5 });    break;
        case OP_6:                   mainStack_.push_back({ 6 });    break;
        case OP_7:                   mainStack_.push_back({ 7 });    break;
        case OP_8:                   mainStack_.push_back({ 8 });    break;
        case OP_9:                   mainStack_.push_back({ 9 });    break;
        case OP_10:                  mainStack_.push_back({ 10 });    break;
        case OP_11:                  mainStack_.push_back({ 11 });    break;
        case OP_12:                  mainStack_.push_back({ 12 });    break;
        case OP_13:                  mainStack_.push_back({ 13 });    break;
        case OP_14:                  mainStack_.push_back({ 14 });    break;
        case OP_15:                  mainStack_.push_back({ 15 });    break;
        case OP_16:                  mainStack_.push_back({ 16 });    break;

        case OP_NOP:                 /* do nothing */   break;

        case OP_VER:                 return false;

        case OP_IF:
        {
            i = processBranch(i, toBool(*p0));
            if (i == instructions_.end())
                return false;
            break;
        }
        case OP_NOTIF:
        {
            i = processBranch(i, !toBool(*p0));
            if (i == instructions_.end())
                return false;
            break;
        }

        case OP_VERIF:               return false;
        case OP_VERNOTIF:            return false;

        case OP_ELSE:
        case OP_ENDIF:
        {
            if (scopeStack_.empty())
                return false;
            i = scopeStack_.top();
            scopeStack_.pop();
            break;
        }
        case OP_VERIFY:              return (toBool(*p0));
        case OP_RETURN:              return false;

            // Stack

        case OP_TOALTSTACK:
        {
            altStack_.push_back(*p0);
            mainStack_.pop_back();
            break;
        }
        case OP_FROMALTSTACK:
        {
            if (altStack_.empty())
                return false;
            mainStack_.push_back(altStack_.back());
            altStack_.pop_back();
            break;
        }
        case OP_2DROP:
        {
            mainStack_.pop_back();
            mainStack_.pop_back();
            break;
        }
        case OP_2DUP:
        {
            std::vector<uint8_t> a = *p1;
            std::vector<uint8_t> b = *p0;
            mainStack_.push_back(*p1);
            mainStack_.push_back(*p0);
            break;
        }
        case OP_3DUP:
        {
            std::vector<uint8_t> a = *p2;
            std::vector<uint8_t> b = *p1;
            std::vector<uint8_t> c = *p0;
            mainStack_.push_back(a);
            mainStack_.push_back(b);
            mainStack_.push_back(c);
            break;
        }
        case OP_2OVER:
        {
            std::vector<uint8_t> a = *p3;
            std::vector<uint8_t> b = *p2;
            mainStack_.push_back(a);
            mainStack_.push_back(b);
            break;
        }
        case OP_2ROT:
        {
            std::rotate(mainStack_.end() - 6, p3, mainStack_.end());
            break;
        }
        case OP_2SWAP:
        {
            std::rotate(p3, p1, mainStack_.end());
            break;
        }
        case OP_IFDUP:
        {
            std::vector<uint8_t> top = *p0;
            if (toBool(top))
                mainStack_.push_back(top);
            break;
        }
        case OP_DEPTH:
        {
            mainStack_.push_back(toElement((int32_t)mainStack_.size()));
            break;
        }
        case OP_DROP:
        {
            mainStack_.pop_back();
            break;
        }
        case OP_DUP:
        {
            std::vector<uint8_t> top = *p0;
            mainStack_.push_back(top);
            break;
        }
        case OP_NIP:
        {
            mainStack_.erase(p1);
            break;
        }
        case OP_OVER:
        {
            std::vector<uint8_t> a = *p1;
            mainStack_.push_back(a);
            break;
        }
        case OP_PICK:
        {
            if (p0->size() > 4)
                return false;
            int32_t n = toInt(*p0);
            if (n < 0 || mainStack_.size() - 2 < (size_t)n)
                return false;
            break;
        }
        case OP_ROLL:
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
        case OP_ROT:
        {
            std::rotate(p2, p1, mainStack_.end());
            break;
        }
        case OP_SWAP:
        {
            std::swap(*p0, *p1);
            break;
        }
        case OP_TUCK:
        {
            std::vector<uint8_t> a = *p0;
            mainStack_.insert(p1, a);
            break;
        }

        // Splice

        case OP_CAT:                 return false;
        case OP_SUBSTR:              return false;
        case OP_LEFT:                return false;
        case OP_RIGHT:               return false;

        case OP_SIZE:
        {
            mainStack_.push_back(toElement((int32_t)p0->size()));
            break;
        }

        // Bitwise logic

        case OP_INVERT:              return false;
        case OP_AND:                 return false;
        case OP_OR:                  return false;
        case OP_XOR:                 return false;

        case OP_EQUAL:
        {
            bool equal = (p0->size() == p1->size() && std::equal(p0->begin(), p0->end(), p1->begin()));
            mainStack_.pop_back();
            mainStack_.back() = toElement(equal);
            break;
        }
        case OP_EQUALVERIFY:
        {
            bool equal = (p0->size() == p1->size() && std::equal(p0->begin(), p0->end(), p1->begin()));
            mainStack_.pop_back();
            mainStack_.back() = toElement(equal);
            if (!equal)
                return false;
            break;
        }

        case OP_RESERVED1:           return false;
        case OP_RESERVED2:           return false;

            // Arithmetic

        case OP_1ADD:
        {
            if (p0->size() > 4)
                return false;
            int32_t result = toInt(*p0) + 1;
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_1SUB:
        {
            if (p0->size() > 4)
                return false;
            int32_t result = toInt(*p0) - 1;
            mainStack_.back() = toElement(result);
            break;
        }

        case OP_2MUL:                return false;
        case OP_2DIV:                return false;

        case OP_NEGATE:
        {
            if (p0->size() > 4)
                return false;
            int32_t result = -toInt(*p0);
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_ABS:
        {
            if (p0->size() > 4)
                return false;
            int32_t result = abs(toInt(*p0));
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_NOT:
        {
            if (p0->size() > 4)
                return false;
            bool result = !toBool(*p0);
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_0NOTEQUAL:
        {
            if (p0->size() > 4)
                return false;
            bool result = toBool(*p0);
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_ADD:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            int32_t result = toInt(*p1) + toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_SUB:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            int32_t result = toInt(*p1) - toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }

        case OP_MUL:                 return false;
        case OP_DIV:                 return false;
        case OP_MOD:                 return false;
        case OP_LSHIFT:              return false;
        case OP_RSHIFT:              return false;

        case OP_BOOLAND:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toBool(*p1) && toBool(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_BOOLOR:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toBool(*p1) || toBool(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_NUMEQUAL:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) == toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_NUMEQUALVERIFY:
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
        case OP_NUMNOTEQUAL:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) != toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_LESSTHAN:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) < toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_GREATERTHAN:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) > toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_LESSTHANOREQUAL:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) <= toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_GREATERTHANOREQUAL:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = toInt(*p1) >= toInt(*p0);
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_MIN:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            bool result = std::min(toInt(*p1), toInt(*p0));
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_MAX:
        {
            if (p0->size() > 4 || p1->size() > 4)
                return false;
            int32_t result = std::max(toInt(*p1), toInt(*p0));
            mainStack_.pop_back();
            mainStack_.back() = toElement(result);
            break;
        }
        case OP_WITHIN:
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

        case OP_RIPEMD160:
        {
            mainStack_.back() = Crypto::ripemd160(*p0);
            break;
        }
        case OP_SHA1:
        {
            mainStack_.back() = Crypto::sha1(*p0);
            break;
        }
        case OP_SHA256:
        {
            mainStack_.back() = Crypto::sha256(*p0);
            break;
        }
        case OP_HASH160:
        {
            mainStack_.back() = Crypto::ripemd160(Crypto::sha256(*p0));
            break;
        }
        case OP_HASH256:
        {
            mainStack_.back() = Crypto::sha256(Crypto::sha256(*p0));
            break;
        }
        case OP_CODESEPARATOR:
        {
            codeSeparator_ = i - 1;
            break;
        }
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

    return mainStack_.empty() ? false : toBool(mainStack_.back());
}

std::string Script::toJson() const
{
    return std::string();
}

std::string Script::toHex() const
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
        else if (op == OP_VERIF || OP_VERNOTIF)
        {
            return false;
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
    return true;
}

bool Script::check() const
{
    for (auto i : instructions_)
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

std::vector<Script::Instruction>::const_iterator Script::findMatchingElse(std::vector<Script::Instruction>::const_iterator start) const
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

Script::Program::iterator Script::processBranch(Program::const_iterator i, bool condition) const
{
    if (condition)
    {
        auto end = findMatchingEndif(i);
        if (end == instructions_.end())
            return end;
        scopeStack_.push(end + 1);
    }
    else
    {
        i = findMatchingElse(i);
        if (i == instructions_.end())
            return i;
        if (i->op == OP_ELSE)
        {
            ++i;
            auto end = findMatchingEndif(i);
            if (end == instructions_.end())
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
