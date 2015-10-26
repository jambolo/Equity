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

static std::vector<uint8_t> toElement(int32_t x)
{
    std::vector<uint8_t> out;
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
        out.push_back((a >>  8) & 0xff);
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

        std::vector<std::vector<uint8_t> >::iterator p0 = (mainStack_.size() > 0) ? mainStack_.end()-1 : mainStack_.end();
        std::vector<std::vector<uint8_t> >::iterator p1 = (mainStack_.size() > 1) ? mainStack_.end()-2 : mainStack_.end();
        std::vector<std::vector<uint8_t> >::iterator p2 = (mainStack_.size() > 2) ? mainStack_.end()-3 : mainStack_.end();
        std::vector<std::vector<uint8_t> >::iterator p3 = (mainStack_.size() > 3) ? mainStack_.end()-4 : mainStack_.end();

        if (op == 0)
        {
            mainStack_.push_back(data);
        }
        switch (op)
        {
        case OP_1NEGATE:             mainStack_.push_back({ 0x81 });  break;
        case OP_RESERVED:            return false;
        case OP_1:                   mainStack_.push_back({  1 });    break;
        case OP_2:                   mainStack_.push_back({  2 });    break;
        case OP_3:                   mainStack_.push_back({  3 });    break;
        case OP_4:                   mainStack_.push_back({  4 });    break;
        case OP_5:                   mainStack_.push_back({  5 });    break;
        case OP_6:                   mainStack_.push_back({  6 });    break;
        case OP_7:                   mainStack_.push_back({  7 });    break;
        case OP_8:                   mainStack_.push_back({  8 });    break;
        case OP_9:                   mainStack_.push_back({  9 });    break;
        case OP_10:                  mainStack_.push_back({ 10 });    break;
        case OP_11:                  mainStack_.push_back({ 11 });    break;
        case OP_12:                  mainStack_.push_back({ 12 });    break;
        case OP_13:                  mainStack_.push_back({ 13 });    break;
        case OP_14:                  mainStack_.push_back({ 14 });    break;
        case OP_15:                  mainStack_.push_back({ 15 });    break;
        case OP_16:                  mainStack_.push_back({ 16 });    break;
        case OP_NOP:                 /* do nothing */           break;
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
        case OP_VERIFY:              return (!mainStack_.empty() && toBool(*p0));
        case OP_RETURN:              return false;

            // Stack

        case OP_TOALTSTACK:
        {
            if (mainStack_.empty())
                return false;
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
            if (mainStack_.size() < 2)
                return false;
            mainStack_.pop_back();
            mainStack_.pop_back();
            break;
        }
        case OP_2DUP: 
        {
            if (mainStack_.size() < 2)
                return false;
            std::vector<uint8_t> a = *p1;
            std::vector<uint8_t> b = *p0;
            mainStack_.push_back(*p1);
            mainStack_.push_back(*p0);
            break;
        }
        case OP_3DUP:
        {
            if (mainStack_.size() < 3)
                return false;
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
            if (mainStack_.size() < 4)
                return false;
            std::vector<uint8_t> a = *p3;
            std::vector<uint8_t> b = *p2;
            mainStack_.push_back(a);
            mainStack_.push_back(b);
            break;
        }
        case OP_2ROT:
        {
            if (mainStack_.size() < 6)
                return false;
            std::rotate(mainStack_.end() - 6, p3, mainStack_.end());
            break;
        }
        case OP_2SWAP:
        {
            if (mainStack_.size() < 4)
                return false;
            std::rotate(p3, p1, mainStack_.end());
            break;
        }
        case OP_IFDUP:
        {
            if (mainStack_.size() < 1)
                return false;
            std::vector<uint8_t> top = *p0;
            if (toBool(top))
                mainStack_.push_back(top);
            break;
        }
        case OP_DEPTH:
        {
            mainStack_.push_back(toElement(mainStack_.size()));
            break;
        }
        case OP_DROP: 
        {
            if (mainStack_.size() < 1)
                return false;
            mainStack_.pop_back();
            break;
        }
        case OP_DUP:
        {
            if (mainStack_.size() < 1)
                return false;
            std::vector<uint8_t> top = *p0;
            mainStack_.push_back(top);
            break;
        }
        case OP_NIP:
        {
            if (mainStack_.size() < 2)
                return false;
            mainStack_.erasep1;
            break;
        }
        case OP_OVER:
        {
            if (mainStack_.size() < 2)
                return false;
            std::vector<uint8_t> a = *p1;
            mainStack_.push_back(a);
            break;
        }
        case OP_PICK:
        {
            if (mainStack_.size() < 2)  // Enough room for index and 1 element
                return false;
            if (*p0.size() > 4)
                return false;
            int32_t n = toInt(*p0);
            if (n < 0 || mainStack_.size()-2 < (size_t)n)
                return false;
            break;
        }
        case OP_ROLL:
        {
            if (mainStack_.size() < 2)  // Enough room for index and 1 element
                return false;
            if (*p0.size() > 4)
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
            if (mainStack_.size() < 3)
                return false;
            std::rotate(p2, p1, mainStack_.end());
            break;
        }
        case OP_SWAP:
        {
            if (mainStack_.size() < 2)
                return false;
            std::swap(*p0, *p1);
            break;
        }
        case OP_TUCK:
        {
            if (mainStack_.size() < 2)
                return false;
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
            if (mainStack_.size() < 1)
                return false;
            mainStack_.push_back(toElement(p0->size()));
            break;
        }

            // Bitwise logic

        case OP_INVERT:              return false;
        case OP_AND:                 return false;
        case OP_OR:                  return false;
        case OP_XOR:                 return false;
        case OP_EQUAL:               break;
        {
            if (mainStack_.size() < 2)
                return false;
            bool equal = (p0->size() == p1->size() && std::equal(p0->begin(), p0->end(), p1->begin()));
            mainStack_.pop_back();
            mainStack_.pop_back();
            if (equal)
            {
                mainStack_.push_back({ 1 });
            }
            else
            {
                mainStack_.push_back(std::vector<uint8_t>());
            }
            break;
        }
        case OP_EQUALVERIFY:         break;
        {
            if (mainStack_.size() < 2)
                return false;
            bool equal = (p0->size() == p1->size() && std::equal(p0->begin(), p0->end(), p1->begin()));
            mainStack_.pop_back();
            mainStack_.pop_back();
            if (equal)
            {
                mainStack_.push_back({ 1 });
            }
            else
            {
                mainStack_.push_back(std::vector<uint8_t>());
                return false;
            }
            break;
        }
        case OP_RESERVED1:           return false;
        case OP_RESERVED2:           return false;

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

Script::Program::const_iterator Script::processBranch(Program::const_iterator i, bool condition) const
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
