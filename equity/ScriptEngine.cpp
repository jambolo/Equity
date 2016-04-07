#include "ScriptEngine.h"

#include "Script.h"

#include "crypto/Ripemd.h"
#include "crypto/Sha1.h"
#include "crypto/Sha256.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

using namespace Equity;

static bool toBool(std::vector<uint8_t> const & data)
{
    // An empty vector represents FALSE
    if (data.empty())
    {
        return false;
    }

    // Anything not equal to [00 00 ...] or [80 00 ...] is TRUE
    if (data.front() != 0 && data.front() != 0x80)
    {
        return true;
    }
    for (auto x : data)
    {
        if (x != 0)
        {
            return true;
        }
    }

    // Must be [00 00 ...] or [80 00 ...], so FALSE
    return false;
}

static int32_t toInt(std::vector<uint8_t> const & data)
{
    if (data.empty())
    {
        return 0;
    }

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
    {
        return out;
    }

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
    {
        out.back() = out.back() | 0x80;
    }

    return out;
}

static std::vector<uint8_t> toElement(bool x)
{
    if (x)
    {
        return std::vector<uint8_t>(1, 1);
    }
    else
    {
        return std::vector<uint8_t>(1, 1);
    }
}

bool ScriptEngine::run(Script const & script)
{
    if (!script.valid())
    {
        return false;
    }

    codeSeparator_ = 0;

    Program instructions = script.instructions();

    Program::const_iterator i = instructions.begin();
    while (i != instructions.end())
    {
        int op = i->op();
        std::vector<uint8_t> const & data = i->data();
        size_t offset = i->location();
        Instruction::Description const & description = i->description();
        ++i;

        if (mainStack_.size() < description.minArgs)
        {
            return false;
        }

        // Shortcuts for the first 4 elements on the stack
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
            case Instruction::OP_1NEGATE:             mainStack_.push_back(std::vector<uint8_t>(1, 0x81));  break;

            case Instruction::OP_RESERVED:            return false;

            case Instruction::OP_1:                   mainStack_.push_back(std::vector<uint8_t>(1,  1));    break;
            case Instruction::OP_2:                   mainStack_.push_back(std::vector<uint8_t>(1,  2));    break;
            case Instruction::OP_3:                   mainStack_.push_back(std::vector<uint8_t>(1,  3));    break;
            case Instruction::OP_4:                   mainStack_.push_back(std::vector<uint8_t>(1,  4));    break;
            case Instruction::OP_5:                   mainStack_.push_back(std::vector<uint8_t>(1,  5));    break;
            case Instruction::OP_6:                   mainStack_.push_back(std::vector<uint8_t>(1,  6));    break;
            case Instruction::OP_7:                   mainStack_.push_back(std::vector<uint8_t>(1,  7));    break;
            case Instruction::OP_8:                   mainStack_.push_back(std::vector<uint8_t>(1,  8));    break;
            case Instruction::OP_9:                   mainStack_.push_back(std::vector<uint8_t>(1,  9));    break;
            case Instruction::OP_10:                  mainStack_.push_back(std::vector<uint8_t>(1, 10));    break;
            case Instruction::OP_11:                  mainStack_.push_back(std::vector<uint8_t>(1, 11));    break;
            case Instruction::OP_12:                  mainStack_.push_back(std::vector<uint8_t>(1, 12));    break;
            case Instruction::OP_13:                  mainStack_.push_back(std::vector<uint8_t>(1, 13));    break;
            case Instruction::OP_14:                  mainStack_.push_back(std::vector<uint8_t>(1, 14));    break;
            case Instruction::OP_15:                  mainStack_.push_back(std::vector<uint8_t>(1, 15));    break;
            case Instruction::OP_16:                  mainStack_.push_back(std::vector<uint8_t>(1, 16));    break;

            case Instruction::OP_NOP: /* do nothing */ break;

            case Instruction::OP_VER:                 return false;

            case Instruction::OP_IF:
            {
                i = processBranch(instructions, i, toBool(*p0));
                if (i == instructions.end())
                {
                    return false;
                }
                break;
            }
            case Instruction::OP_NOTIF:
            {
                i = processBranch(instructions, i, !toBool(*p0));
                if (i == instructions.end())
                {
                    return false;
                }
                break;
            }

            case Instruction::OP_ELSE:
            case Instruction::OP_ENDIF:
            {
                if (scopeStack_.empty())
                {
                    return false;
                }
                i = scopeStack_.top();
                scopeStack_.pop();
                break;
            }
            case Instruction::OP_VERIFY:
            {
                bool verified = toBool(*p0);
                mainStack_.pop_back();
                return verified;
            }
            case Instruction::OP_RETURN:              return false;

            // Stack

            case Instruction::OP_TOALTSTACK:
            {
                altStack_.push_back(*p0);
                mainStack_.pop_back();
                break;
            }
            case Instruction::OP_FROMALTSTACK:
            {
                if (altStack_.empty())
                {
                    return false;
                }
                mainStack_.push_back(altStack_.back());
                altStack_.pop_back();
                break;
            }
            case Instruction::OP_2DROP:
            {
                mainStack_.pop_back();
                mainStack_.pop_back();
                break;
            }
            case Instruction::OP_2DUP:
            {
                std::vector<uint8_t> a = *p1;
                std::vector<uint8_t> b = *p0;
                mainStack_.push_back(*p1);
                mainStack_.push_back(*p0);
                break;
            }
            case Instruction::OP_3DUP:
            {
                std::vector<uint8_t> a = *p2;
                std::vector<uint8_t> b = *p1;
                std::vector<uint8_t> c = *p0;
                mainStack_.push_back(a);
                mainStack_.push_back(b);
                mainStack_.push_back(c);
                break;
            }
            case Instruction::OP_2OVER:
            {
                std::vector<uint8_t> a = *p3;
                std::vector<uint8_t> b = *p2;
                mainStack_.push_back(a);
                mainStack_.push_back(b);
                break;
            }
            case Instruction::OP_2ROT:
            {
                std::rotate(mainStack_.end() - 6, p3, mainStack_.end());
                break;
            }
            case Instruction::OP_2SWAP:
            {
                std::rotate(p3, p1, mainStack_.end());
                break;
            }
            case Instruction::OP_IFDUP:
            {
                std::vector<uint8_t> top = *p0;
                if (toBool(top))
                {
                    mainStack_.push_back(top);
                }
                break;
            }
            case Instruction::OP_DEPTH:
            {
                mainStack_.push_back(toElement((int32_t)mainStack_.size()));
                break;
            }
            case Instruction::OP_DROP:
            {
                mainStack_.pop_back();
                break;
            }
            case Instruction::OP_DUP:
            {
                std::vector<uint8_t> top = *p0;
                mainStack_.push_back(top);
                break;
            }
            case Instruction::OP_NIP:
            {
                mainStack_.erase(p1);
                break;
            }
            case Instruction::OP_OVER:
            {
                std::vector<uint8_t> a = *p1;
                mainStack_.push_back(a);
                break;
            }
            case Instruction::OP_PICK:
            {
                if (p0->size() > 4)
                {
                    return false;
                }
                int32_t n = toInt(*p0);
                if (n < 0 || mainStack_.size() - 2 < (size_t)n)
                {
                    return false;
                }
                break;
            }
            case Instruction::OP_ROLL:
            {
                if (p0->size() > 4)
                {
                    return false;
                }
                int32_t n = toInt(*p0);
                if (n < 0 || mainStack_.size() - 2 < (size_t)n)
                {
                    return false;
                }
                *p0 = *(p1 - n);
                mainStack_.erase(p1 - n);
                break;
            }
            case Instruction::OP_ROT:
            {
                std::rotate(p2, p1, mainStack_.end());
                break;
            }
            case Instruction::OP_SWAP:
            {
                std::swap(*p0, *p1);
                break;
            }
            case Instruction::OP_TUCK:
            {
                std::vector<uint8_t> a = *p0;
                mainStack_.insert(p1, a);
                break;
            }

            // Splice

            case Instruction::OP_SIZE:
            {
                mainStack_.push_back(toElement((int32_t)p0->size()));
                break;
            }

            // Bitwise logic

            case Instruction::OP_EQUAL:
            {
                bool equal = (*p0 == *p1);
                mainStack_.pop_back();
                mainStack_.back() = toElement(equal);
                break;
            }
            case Instruction::OP_EQUALVERIFY:
            {
                bool equal = (*p0 == *p1);
                mainStack_.pop_back();
                mainStack_.pop_back();
                if (!equal)
                {
                    return false;
                }
                break;
            }

            // Arithmetic

            case Instruction::OP_1ADD:
            {
                if (p0->size() > 4)
                {
                    return false;
                }
                int32_t result = toInt(*p0) + 1;
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_1SUB:
            {
                if (p0->size() > 4)
                {
                    return false;
                }
                int32_t result = toInt(*p0) - 1;
                mainStack_.back() = toElement(result);
                break;
            }

            case Instruction::OP_NEGATE:
            {
                if (p0->size() > 4)
                {
                    return false;
                }
                int32_t result = -toInt(*p0);
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_ABS:
            {
                if (p0->size() > 4)
                {
                    return false;
                }
                int32_t result = abs(toInt(*p0));
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_NOT:
            {
                if (p0->size() > 4)
                {
                    return false;
                }
                bool result = !toBool(*p0);
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_0NOTEQUAL:
            {
                if (p0->size() > 4)
                {
                    return false;
                }
                bool result = toBool(*p0);
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_ADD:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                int32_t result = toInt(*p1) + toInt(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_SUB:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                int32_t result = toInt(*p1) - toInt(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }

            case Instruction::OP_BOOLAND:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toBool(*p1) && toBool(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_BOOLOR:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toBool(*p1) || toBool(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_NUMEQUAL:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toInt(*p1) == toInt(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_NUMEQUALVERIFY:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toInt(*p1) == toInt(*p0);
                mainStack_.pop_back();
                mainStack_.pop_back();
                if (!result)
                {
                    return false;
                }
                break;
            }
            case Instruction::OP_NUMNOTEQUAL:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toInt(*p1) != toInt(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_LESSTHAN:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toInt(*p1) < toInt(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_GREATERTHAN:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toInt(*p1) > toInt(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_LESSTHANOREQUAL:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toInt(*p1) <= toInt(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_GREATERTHANOREQUAL:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                bool result = toInt(*p1) >= toInt(*p0);
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_MIN:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                int32_t result = std::min(toInt(*p1), toInt(*p0));
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_MAX:
            {
                if (p0->size() > 4 || p1->size() > 4)
                {
                    return false;
                }
                int32_t result = std::max(toInt(*p1), toInt(*p0));
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }
            case Instruction::OP_WITHIN:
            {
                if (p0->size() > 4 || p1->size() > 4 || p2->size() > 4)
                {
                    return false;
                }
                int32_t x = toInt(*p1);
                bool result = toInt(*p2) <= x && x < toInt(*p2);
                mainStack_.pop_back();
                mainStack_.pop_back();
                mainStack_.back() = toElement(result);
                break;
            }

            // Crypto

            case Instruction::OP_RIPEMD160:
            {
                Crypto::Ripemd160Hash hash = Crypto::ripemd160(*p0);
                mainStack_.back().assign(hash.begin(), hash.end());
                break;
            }
            case Instruction::OP_SHA1:
            {
                Crypto::Sha1Hash hash = Crypto::sha1(*p0);
                mainStack_.back().assign(hash.begin(), hash.end());
                break;
            }
            case Instruction::OP_SHA256:
            {
                Crypto::Sha256Hash hash = Crypto::sha256(*p0);
                mainStack_.back().assign(hash.begin(), hash.end());
                break;
            }
            case Instruction::OP_HASH160:
            {
                Crypto::Ripemd160Hash hash = Crypto::ripemd160(Crypto::sha256(*p0));
                mainStack_.back().assign(hash.begin(), hash.end());
                break;
            }
            case Instruction::OP_HASH256:
            {
                Crypto::Sha256Hash hash = Crypto::doubleSha256(*p0);
                mainStack_.back().assign(hash.begin(), hash.end());
                break;
            }
            case Instruction::OP_CODESEPARATOR:
            {
                codeSeparator_ = offset;
                break;
            }
            case Instruction::OP_CHECKSIG:            break;
            case Instruction::OP_CHECKSIGVERIFY:      break;
            case Instruction::OP_CHECKMULTISIG:       break;
            case Instruction::OP_CHECKMULTISIGVERIFY: break;

            case Instruction::OP_NOP1:                break;

            case Instruction::OP_CHECKLOCKTIMEVERIFY: break;

            case Instruction::OP_NOP3:                break;
            case Instruction::OP_NOP4:                break;
            case Instruction::OP_NOP5:                break;
            case Instruction::OP_NOP6:                break;
            case Instruction::OP_NOP7:                break;
            case Instruction::OP_NOP8:                break;
            case Instruction::OP_NOP9:                break;
            case Instruction::OP_NOP10:               break;

            default:
                // All other instructions are invalid or mark the transaction as invalid. Note: These op codes
                // should have been detected when the script was parsed.
                assert(false);
                return false;
        }
    }

    return mainStack_.empty() ? false : toBool(mainStack_.back());
}

Program::const_iterator ScriptEngine::findMatchingElse(Program const & instructions, Program::const_iterator start) const
{
    auto i = start;
    while (i != instructions.end())
    {
        if (i->op() == Instruction::OP_IF || i->op() == Instruction::OP_NOTIF)
        {
            ++i;
            i = findMatchingEndif(instructions, i);
            if (i != instructions.end())
            {
                ++i;
            }
        }
        else if (i->op() == Instruction::OP_ELSE || i->op() == Instruction::OP_ENDIF)
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

Program::const_iterator ScriptEngine::findMatchingEndif(Program const & instructions, Program::const_iterator start) const
{
    auto i = start;
    while (i != instructions.end())
    {
        if (i->op() == Instruction::OP_IF || i->op() == Instruction::OP_NOTIF)
        {
            ++i;
            i = findMatchingEndif(instructions, i);
            if (i != instructions.end())
            {
                ++i;
            }
        }
        else if (i->op() == Instruction::OP_ENDIF)
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

Program::const_iterator ScriptEngine::processBranch(Program const & instructions, Program::const_iterator i, bool condition)
{
    if (condition)
    {
        auto end = findMatchingEndif(instructions, i);
        if (end == instructions.end())
        {
            return end;
        }
        scopeStack_.push(end + 1);
    }
    else
    {
        i = findMatchingElse(instructions, i);
        if (i == instructions.end())
        {
            return i;
        }
        if (i->op() == Instruction::OP_ELSE)
        {
            ++i;
            auto end = findMatchingEndif(instructions, i);
            if (end == instructions.end())
            {
                return end;
            }
            scopeStack_.push(end + 1);
        }
        else
        {
            scopeStack_.push(i + 1);
        }
    }
    return i;
}
