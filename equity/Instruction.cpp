#include "Instruction.h"

#include "utility/Serialize.h"

#include <cassert>

using namespace Equity;

Instruction::Description const Instruction::DESCRIPTIONS[Instruction::NUMBER_OF_INSTRUCTIONS] =
{
    { OP_0,                     "0",                    0, true }, /* 00 */
    { 0x01,                     "PUSHDATA",             0, true }, /* 01 */
    { 0x02,                     "PUSHDATA",             0, true }, /* 02 */
    { 0x03,                     "PUSHDATA",             0, true }, /* 03 */
    { 0x04,                     "PUSHDATA",             0, true }, /* 04 */
    { 0x05,                     "PUSHDATA",             0, true }, /* 05 */
    { 0x06,                     "PUSHDATA",             0, true }, /* 06 */
    { 0x07,                     "PUSHDATA",             0, true }, /* 07 */
    { 0x08,                     "PUSHDATA",             0, true }, /* 08 */
    { 0x09,                     "PUSHDATA",             0, true }, /* 09 */
    { 0x0a,                     "PUSHDATA",             0, true }, /* 0a */
    { 0x0b,                     "PUSHDATA",             0, true }, /* 0b */
    { 0x0c,                     "PUSHDATA",             0, true }, /* 0c */
    { 0x0d,                     "PUSHDATA",             0, true }, /* 0d */
    { 0x0e,                     "PUSHDATA",             0, true }, /* 0e */
    { 0x0f,                     "PUSHDATA",             0, true }, /* 0f */
    { 0x10,                     "PUSHDATA",             0, true }, /* 10 */
    { 0x11,                     "PUSHDATA",             0, true }, /* 11 */
    { 0x12,                     "PUSHDATA",             0, true }, /* 12 */
    { 0x13,                     "PUSHDATA",             0, true }, /* 13 */
    { 0x14,                     "PUSHDATA",             0, true }, /* 14 */
    { 0x15,                     "PUSHDATA",             0, true }, /* 15 */
    { 0x16,                     "PUSHDATA",             0, true }, /* 16 */
    { 0x17,                     "PUSHDATA",             0, true }, /* 17 */
    { 0x18,                     "PUSHDATA",             0, true }, /* 18 */
    { 0x19,                     "PUSHDATA",             0, true }, /* 19 */
    { 0x1a,                     "PUSHDATA",             0, true }, /* 1a */
    { 0x1b,                     "PUSHDATA",             0, true }, /* 1b */
    { 0x1c,                     "PUSHDATA",             0, true }, /* 1c */
    { 0x1d,                     "PUSHDATA",             0, true }, /* 1d */
    { 0x1e,                     "PUSHDATA",             0, true }, /* 1e */
    { 0x1f,                     "PUSHDATA",             0, true }, /* 1f */
    { 0x20,                     "PUSHDATA",             0, true }, /* 20 */
    { 0x21,                     "PUSHDATA",             0, true }, /* 21 */
    { 0x22,                     "PUSHDATA",             0, true }, /* 22 */
    { 0x23,                     "PUSHDATA",             0, true }, /* 23 */
    { 0x24,                     "PUSHDATA",             0, true }, /* 24 */
    { 0x25,                     "PUSHDATA",             0, true }, /* 25 */
    { 0x26,                     "PUSHDATA",             0, true }, /* 26 */
    { 0x27,                     "PUSHDATA",             0, true }, /* 27 */
    { 0x28,                     "PUSHDATA",             0, true }, /* 28 */
    { 0x29,                     "PUSHDATA",             0, true }, /* 29 */
    { 0x2a,                     "PUSHDATA",             0, true }, /* 2a */
    { 0x2b,                     "PUSHDATA",             0, true }, /* 2b */
    { 0x2c,                     "PUSHDATA",             0, true }, /* 2c */
    { 0x2d,                     "PUSHDATA",             0, true }, /* 2d */
    { 0x2e,                     "PUSHDATA",             0, true }, /* 2e */
    { 0x2f,                     "PUSHDATA",             0, true }, /* 2f */
    { 0x30,                     "PUSHDATA",             0, true }, /* 30 */
    { 0x31,                     "PUSHDATA",             0, true }, /* 31 */
    { 0x32,                     "PUSHDATA",             0, true }, /* 32 */
    { 0x33,                     "PUSHDATA",             0, true }, /* 33 */
    { 0x34,                     "PUSHDATA",             0, true }, /* 34 */
    { 0x35,                     "PUSHDATA",             0, true }, /* 35 */
    { 0x36,                     "PUSHDATA",             0, true }, /* 36 */
    { 0x37,                     "PUSHDATA",             0, true }, /* 37 */
    { 0x38,                     "PUSHDATA",             0, true }, /* 38 */
    { 0x39,                     "PUSHDATA",             0, true }, /* 39 */
    { 0x3a,                     "PUSHDATA",             0, true }, /* 3a */
    { 0x3b,                     "PUSHDATA",             0, true }, /* 3b */
    { 0x3c,                     "PUSHDATA",             0, true }, /* 3c */
    { 0x3d,                     "PUSHDATA",             0, true }, /* 3d */
    { 0x3e,                     "PUSHDATA",             0, true }, /* 3e */
    { 0x3f,                     "PUSHDATA",             0, true }, /* 3f */
    { 0x40,                     "PUSHDATA",             0, true }, /* 40 */
    { 0x41,                     "PUSHDATA",             0, true }, /* 41 */
    { 0x42,                     "PUSHDATA",             0, true }, /* 42 */
    { 0x43,                     "PUSHDATA",             0, true }, /* 43 */
    { 0x44,                     "PUSHDATA",             0, true }, /* 44 */
    { 0x45,                     "PUSHDATA",             0, true }, /* 45 */
    { 0x46,                     "PUSHDATA",             0, true }, /* 46 */
    { 0x47,                     "PUSHDATA",             0, true }, /* 47 */
    { 0x48,                     "PUSHDATA",             0, true }, /* 48 */
    { 0x49,                     "PUSHDATA",             0, true }, /* 49 */
    { 0x4a,                     "PUSHDATA",             0, true }, /* 4a */
    { 0x4b,                     "PUSHDATA",             0, true }, /* 4b */
    { OP_PUSHDATA1,             "PUSHDATA1",            0, true }, /* 4c */
    { OP_PUSHDATA2,             "PUSHDATA2",            0, true }, /* 4d */
    { OP_PUSHDATA4,             "PUSHDATA4",            0, true }, /* 4e */
    { OP_1NEGATE,               "1NEGATE",              0, true }, /* 4f */
    { OP_RESERVED,              "RESERVED",             0, true }, /* 50 */
    { OP_1,                     "1",                    0, true }, /* 51 */
    { OP_2,                     "2",                    0, true }, /* 52 */
    { OP_3,                     "3",                    0, true }, /* 53 */
    { OP_4,                     "4",                    0, true }, /* 54 */
    { OP_5,                     "5",                    0, true }, /* 55 */
    { OP_6,                     "6",                    0, true }, /* 56 */
    { OP_7,                     "7",                    0, true }, /* 57 */
    { OP_8,                     "8",                    0, true }, /* 58 */
    { OP_9,                     "9",                    0, true }, /* 59 */
    { OP_10,                    "10",                   0, true }, /* 5a */
    { OP_11,                    "11",                   0, true }, /* 5b */
    { OP_12,                    "12",                   0, true }, /* 5c */
    { OP_13,                    "13",                   0, true }, /* 5d */
    { OP_14,                    "14",                   0, true }, /* 5e */
    { OP_15,                    "15",                   0, true }, /* 5f */
    { OP_16,                    "16",                   0, true }, /* 60 */
    { OP_NOP,                   "NOP",                  0, true }, /* 61 */
    { OP_VER,                   "VER",                  0, true }, /* 62 */
    { OP_IF,                    "IF",                   1, true }, /* 63 */
    { OP_NOTIF,                 "NOTIF",                1, true }, /* 64 */
    { OP_VERIF,                 "VERIF",                0, false }, /* 65 */
    { OP_VERNOTIF,              "VERNOTIF",             0, false }, /* 66 */
    { OP_ELSE,                  "ELSE",                 0, true }, /* 67 */
    { OP_ENDIF,                 "ENDIF",                0, true }, /* 68 */
    { OP_VERIFY,                "VERIFY",               1, true }, /* 69 */
    { OP_RETURN,                "RETURN",               0, true }, /* 6a */
    { OP_TOALTSTACK,            "TOALTSTACK",           1, true }, /* 6b */
    { OP_FROMALTSTACK,          "FROMALTSTACK",         0, true }, /* 6c */
    { OP_2DROP,                 "2DROP",                2, true }, /* 6d */
    { OP_2DUP,                  "2DUP",                 2, true }, /* 6e */
    { OP_3DUP,                  "3DUP",                 3, true }, /* 6f */
    { OP_2OVER,                 "2OVER",                4, true }, /* 70 */
    { OP_2ROT,                  "2ROT",                 6, true }, /* 71 */
    { OP_2SWAP,                 "2SWAP",                4, true }, /* 72 */
    { OP_IFDUP,                 "IFDUP",                1, true }, /* 73 */
    { OP_DEPTH,                 "DEPTH",                0, true }, /* 74 */
    { OP_DROP,                  "DROP",                 1, true }, /* 75 */
    { OP_DUP,                   "DUP",                  1, true }, /* 76 */
    { OP_NIP,                   "NIP",                  2, true }, /* 77 */
    { OP_OVER,                  "OVER",                 2, true }, /* 78 */
    { OP_PICK,                  "PICK",                 2, true }, /* 79 */
    { OP_ROLL,                  "ROLL",                 2, true }, /* 7a */
    { OP_ROT,                   "ROT",                  3, true }, /* 7b */
    { OP_SWAP,                  "SWAP",                 2, true }, /* 7c */
    { OP_TUCK,                  "TUCK",                 2, true }, /* 7d */
    { 0x7e,                     "(disabled)",           0, false }, /* 7e */
    { 0x7f,                     "(disabled)",           0, false }, /* 7f */
    { 0x80,                     "(disabled)",           0, false }, /* 80 */
    { 0x81,                     "(disabled)",           0, false }, /* 81 */
    { OP_SIZE,                  "SIZE",                 1, true }, /* 82 */
    { 0x83,                     "(disabled)",           0, false }, /* 83 */
    { 0x84,                     "(disabled)",           0, false }, /* 84 */
    { 0x85,                     "(disabled)",           0, false }, /* 85 */
    { 0x86,                     "(disabled)",           0, false }, /* 86 */
    { OP_EQUAL,                 "EQUAL",                2, true }, /* 87 */
    { OP_EQUALVERIFY,           "EQUALVERIFY",          2, true }, /* 88 */
    { OP_RESERVED1,             "RESERVED1",            0, true }, /* 89 */
    { OP_RESERVED2,             "RESERVED2",            0, true }, /* 8a */
    { OP_1ADD,                  "1ADD",                 1, true }, /* 8b */
    { OP_1SUB,                  "1SUB",                 1, true }, /* 8c */
    { 0x8d,                     "(disabled)",           0, false }, /* 8d */
    { 0x8e,                     "(disabled)",           0, false }, /* 8e */
    { OP_NEGATE,                "NEGATE",               1, true }, /* 8f */
    { OP_ABS,                   "ABS",                  1, true }, /* 90 */
    { OP_NOT,                   "NOT",                  1, true }, /* 91 */
    { OP_0NOTEQUAL,             "0NOTEQUAL",            1, true }, /* 92 */
    { OP_ADD,                   "ADD",                  2, true }, /* 93 */
    { OP_SUB,                   "SUB",                  2, true }, /* 94 */
    { 0x95,                     "(disabled)",           0, false }, /* 95 */
    { 0x96,                     "(disabled)",           0, false }, /* 96 */
    { 0x97,                     "(disabled)",           0, false }, /* 97 */
    { 0x98,                     "(disabled)",           0, false }, /* 98 */
    { 0x99,                     "(disabled)",           0, false }, /* 99 */
    { OP_BOOLAND,               "BOOLAND",              2, true }, /* 9a */
    { OP_BOOLOR,                "BOOLOR",               2, true }, /* 9b */
    { OP_NUMEQUAL,              "NUMEQUAL",             2, true }, /* 9c */
    { OP_NUMEQUALVERIFY,        "NUMEQUALVERIFY",       2, true }, /* 9d */
    { OP_NUMNOTEQUAL,           "NUMNOTEQUAL",          2, true }, /* 9e */
    { OP_LESSTHAN,              "LESSTHAN",             2, true }, /* 9f */
    { OP_GREATERTHAN,           "GREATERTHAN",          2, true }, /* a0 */
    { OP_LESSTHANOREQUAL,       "LESSTHANOREQUAL",      2, true }, /* a1 */
    { OP_GREATERTHANOREQUAL,    "GREATERTHANOREQUAL",   2, true }, /* a2 */
    { OP_MIN,                   "MIN",                  2, true }, /* a3 */
    { OP_MAX,                   "MAX",                  2, true }, /* a4 */
    { OP_WITHIN,                "WITHIN",               3, true }, /* a5 */
    { OP_RIPEMD160,             "RIPEMD160",            1, true }, /* a6 */
    { OP_SHA1,                  "SHA1",                 1, true }, /* a7 */
    { OP_SHA256,                "SHA256",               1, true }, /* a8 */
    { OP_HASH160,               "HASH160",              1, true }, /* a9 */
    { OP_HASH256,               "HASH256",              1, true }, /* aa */
    { OP_CODESEPARATOR,         "CODESEPARATOR",        0, true }, /* ab */
    { OP_CHECKSIG,              "CHECKSIG",             2, true }, /* ac */
    { OP_CHECKSIGVERIFY,        "CHECKSIGVERIFY",       2, true }, /* ad */
    { OP_CHECKMULTISIG,         "CHECKMULTISIG",        3, true }, /* ae */
    { OP_CHECKMULTISIGVERIFY,   "CHECKMULTISIGVERIFY",  3, true }, /* af */
    { OP_NOP1,                  "NOP1",                 0, true }, /* b0 */
    { OP_CHECKLOCKTIMEVERIFY,   "CHECKLOCKTIMEVERIFY",  0, true }, /* b1 */
    { OP_NOP3,                  "NOP3",                 0, true }, /* b2 */
    { OP_NOP4,                  "NOP4",                 0, true }, /* b3 */
    { OP_NOP5,                  "NOP5",                 0, true }, /* b4 */
    { OP_NOP6,                  "NOP6",                 0, true }, /* b5 */
    { OP_NOP7,                  "NOP7",                 0, true }, /* b6 */
    { OP_NOP8,                  "NOP8",                 0, true }, /* b7 */
    { OP_NOP9,                  "NOP9",                 0, true }, /* b8 */
    { OP_NOP10,                 "NOP10",                0, true }, /* b9 */
    { 0xba,                     "(invalid)",            0, false }, /* ba */
    { 0xbb,                     "(invalid)",            0, false }, /* bb */
    { 0xbc,                     "(invalid)",            0, false }, /* bc */
    { 0xbd,                     "(invalid)",            0, false }, /* bd */
    { 0xbe,                     "(invalid)",            0, false }, /* be */
    { 0xbf,                     "(invalid)",            0, false }, /* bf */
    { 0xc0,                     "(invalid)",            0, false }, /* c0 */
    { 0xc1,                     "(invalid)",            0, false }, /* c1 */
    { 0xc2,                     "(invalid)",            0, false }, /* c2 */
    { 0xc3,                     "(invalid)",            0, false }, /* c3 */
    { 0xc4,                     "(invalid)",            0, false }, /* c4 */
    { 0xc5,                     "(invalid)",            0, false }, /* c5 */
    { 0xc6,                     "(invalid)",            0, false }, /* c6 */
    { 0xc7,                     "(invalid)",            0, false }, /* c7 */
    { 0xc8,                     "(invalid)",            0, false }, /* c8 */
    { 0xc9,                     "(invalid)",            0, false }, /* c9 */
    { 0xca,                     "(invalid)",            0, false }, /* ca */
    { 0xcb,                     "(invalid)",            0, false }, /* cb */
    { 0xcc,                     "(invalid)",            0, false }, /* cc */
    { 0xcd,                     "(invalid)",            0, false }, /* cd */
    { 0xce,                     "(invalid)",            0, false }, /* ce */
    { 0xcf,                     "(invalid)",            0, false }, /* cf */
    { 0xd0,                     "(invalid)",            0, false }, /* d0 */
    { 0xd1,                     "(invalid)",            0, false }, /* d1 */
    { 0xd2,                     "(invalid)",            0, false }, /* d2 */
    { 0xd3,                     "(invalid)",            0, false }, /* d3 */
    { 0xd4,                     "(invalid)",            0, false }, /* d4 */
    { 0xd5,                     "(invalid)",            0, false }, /* d5 */
    { 0xd6,                     "(invalid)",            0, false }, /* d6 */
    { 0xd7,                     "(invalid)",            0, false }, /* d7 */
    { 0xd8,                     "(invalid)",            0, false }, /* d8 */
    { 0xd9,                     "(invalid)",            0, false }, /* d9 */
    { 0xda,                     "(invalid)",            0, false }, /* da */
    { 0xdb,                     "(invalid)",            0, false }, /* db */
    { 0xdc,                     "(invalid)",            0, false }, /* dc */
    { 0xdd,                     "(invalid)",            0, false }, /* dd */
    { 0xde,                     "(invalid)",            0, false }, /* de */
    { 0xdf,                     "(invalid)",            0, false }, /* df */
    { 0xe0,                     "(invalid)",            0, false }, /* e0 */
    { 0xe1,                     "(invalid)",            0, false }, /* e1 */
    { 0xe2,                     "(invalid)",            0, false }, /* e2 */
    { 0xe3,                     "(invalid)",            0, false }, /* e3 */
    { 0xe4,                     "(invalid)",            0, false }, /* e4 */
    { 0xe5,                     "(invalid)",            0, false }, /* e5 */
    { 0xe6,                     "(invalid)",            0, false }, /* e6 */
    { 0xe7,                     "(invalid)",            0, false }, /* e7 */
    { 0xe8,                     "(invalid)",            0, false }, /* e8 */
    { 0xe9,                     "(invalid)",            0, false }, /* e9 */
    { 0xea,                     "(invalid)",            0, false }, /* ea */
    { 0xeb,                     "(invalid)",            0, false }, /* eb */
    { 0xec,                     "(invalid)",            0, false }, /* ec */
    { 0xed,                     "(invalid)",            0, false }, /* ed */
    { 0xee,                     "(invalid)",            0, false }, /* ee */
    { 0xef,                     "(invalid)",            0, false }, /* ef */
    { 0xf0,                     "(invalid)",            0, false }, /* f0 */
    { 0xf1,                     "(invalid)",            0, false }, /* f1 */
    { 0xf2,                     "(invalid)",            0, false }, /* f2 */
    { 0xf3,                     "(invalid)",            0, false }, /* f3 */
    { 0xf4,                     "(invalid)",            0, false }, /* f4 */
    { 0xf5,                     "(invalid)",            0, false }, /* f5 */
    { 0xf6,                     "(invalid)",            0, false }, /* f6 */
    { 0xf7,                     "(invalid)",            0, false }, /* f7 */
    { 0xf8,                     "(invalid)",            0, false }, /* f8 */
    { 0xf9,                     "(invalid)",            0, false }, /* f9 */
    { 0xfa,                     "(invalid)",            0, false }, /* fa */
    { 0xfb,                     "(invalid)",            0, false }, /* fb */
    { 0xfc,                     "(invalid)",            0, false }, /* fc */
    { 0xfd,                     "(invalid)",            0, false }, /* fd */
    { 0xfe,                     "(invalid)",            0, false }, /* fe */
    { OP_INVALID,               "(invalid)",            0, false }  /* ff */
};

Instruction::Instruction(int op, std::vector<uint8_t> const & data /* = std::vector<uint8_t>()*/, size_t location /*
                                                                                                                     =
                                                                                                                     0*/)
    : op_(op)
    , data_(data)
    , location_(location)
    , size_(1)
{
    // Validate the opcode and override the opcode if necessary
    assert(op >= 0 && op < NUMBER_OF_INSTRUCTIONS);
    if (op < 0 || op >= NUMBER_OF_INSTRUCTIONS)
    {
        op_ = op = OP_INVALID;
    }

    valid_ = DESCRIPTIONS[op].valid;

    if (!valid_)
    {
        return;
    }

    // Validate data size and override the validity flag if necessary
    if (op >= 0x01 && op <= OP_PUSHDATA4)
    {
        if (data.empty())
        {
            valid_ = false;
        }
        else if (op >= 0x01 && op <= 0x4b && data.size() != op)
        {
            valid_ = false;
        }
        else if (op == OP_PUSHDATA1)
        {
            size_ += 1;
            if (data.size() > 0xffU)
            {
                valid_ = false;
            }
        }
        else if (op == OP_PUSHDATA2)
        {
            size_ += 2;
            if (data.size() > 0xffffU)
            {
                valid_ = false;
            }
        }
        else if (op == OP_PUSHDATA4)
        {
            size_ += 4;
            if (data.size() > 0xffffffffU)
            {
                valid_ = false;
            }
        }
    }
    else
    {
        if (!data.empty())
        {
            valid_ = false;
        }
    }

    if (!valid_)
    {
        return;
    }

    size_ += data.size();
}

Instruction::Instruction(uint8_t const * & in, size_t & size, size_t location)
    : op_(OP_INVALID)
    , location_(location)
    , size_(0)
    , valid_(false)
{
    // Get the opcode
    op_ = Utility::deserialize<uint8_t>(in, size);
    size_ += 1;

    assert(DESCRIPTIONS[op_].value == op_);    // Sanity check

    // If the opcode is not valid, then abort
    if (!DESCRIPTIONS[op_].valid)
    {
        throw Utility::DeserializationError();
    }

    // Certain instructions contain data
    if (op_ >= 0x01 && op_ <= OP_PUSHDATA4)
    {
        size_t count;
        if (op_ >= 0x01 && op_ <= 0x4b)
        {
            count = op_;
        }
        else if (op_ == OP_PUSHDATA1)
        {
            count = Utility::deserialize<uint8_t>(in, size);
            size_ += 1;
        }
        else if (op_ == OP_PUSHDATA2)
        {
            count = Utility::deserialize<uint16_t>(in, size);
            size_ += 2;
        }
        else
        {
            assert(op_ == OP_PUSHDATA4);
            count = Utility::deserialize<uint32_t>(in, size);
            size_ += 4;
        }

        data_ = Utility::deserializeVector<uint8_t>(count, in, size);
        size_ += count;
    }

    valid_ = true;
}

void Instruction::serialize(std::vector<uint8_t> & out) const
{
    if (valid_)
    {
        Utility::serialize<uint8_t>(op_, out);

        if (op_ >= 0x1 && op_ <= 0x4b)
        {
            Utility::serializeVector(data_, out);
        }
        else if (op_ == OP_PUSHDATA1)
        {
            Utility::serialize((uint8_t)data_.size(), out);
            Utility::serializeVector(data_, out);
        }
        else if (op_ == OP_PUSHDATA2)
        {
            Utility::serialize((uint16_t)data_.size(), out);
            Utility::serializeVector(data_, out);
        }
        else if (op_ == OP_PUSHDATA4)
        {
            Utility::serialize((uint32_t)data_.size(), out);
            Utility::serializeVector(data_, out);
        }
    }
    else
    {
        Utility::serialize<uint8_t>(OP_INVALID, out);
    }
}

Instruction::Description const & Instruction::getDescription(int opcode)
{
    assert(opcode >= 0 && opcode < NUMBER_OF_INSTRUCTIONS);

    // Just return information about OP_INVALID if the value is not valid.
    if (opcode < 0 || opcode >= NUMBER_OF_INSTRUCTIONS)
    {
        opcode = OP_INVALID;
    }

    assert(DESCRIPTIONS[opcode].value == opcode);     // Sanity check

    return DESCRIPTIONS[opcode];
}
