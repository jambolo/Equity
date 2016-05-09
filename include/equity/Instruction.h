#pragma once

#include "p2p/Serialize.h"
#include <cstdint>
#include <vector>

namespace Equity
{

//! Exception thrown for a script parsing error
class ScriptParsingError : public std::runtime_error
{
public:
    ScriptParsingError() : std::runtime_error("script parsing error") {}
};

//! A script instruction.
//!
//! A script instruction contains an opcode and optional data. It also contains additional information used for branching and
//! validation.

class Instruction : public P2p::Serializable
{
public:

    enum OpCode
    {
        // Constants

        OP_0                    = 0x00, //!< An empty array of bytes is pushed onto the stack. (This is not a no-op: an item is
                                        //!< added to the stack.)
        OP_FALSE = OP_0,

        // data                 = 0x01-0x4b    The next opcode bytes is data to be pushed onto the stack

        OP_PUSHDATA1            = 0x4c, //!< The next byte contains the number of bytes to be pushed onto the stack.
        OP_PUSHDATA2            = 0x4d, //!< The next two bytes contain the number of bytes to be pushed onto the stack.
        OP_PUSHDATA4            = 0x4e, //!< The next four bytes contain the number of bytes to be pushed onto the stack.
        OP_1NEGATE              = 0x4f, //!< The number -1 is pushed onto the stack.
        OP_RESERVED             = 0x50, //!< Transaction is invalid unless occuring in an unexecuted OP_IF branch
        OP_1                    = 0x51, //!< The number 1  is pushed onto the stack.
        OP_TRUE = OP_1,                 //!< The number 1  is pushed onto the stack.
        OP_2                    = 0x52, //!< The number 2  is pushed onto the stack.
        OP_3                    = 0x53, //!< The number 3  is pushed onto the stack.
        OP_4                    = 0x54, //!< The number 4  is pushed onto the stack.
        OP_5                    = 0x55, //!< The number 5  is pushed onto the stack.
        OP_6                    = 0x56, //!< The number 6  is pushed onto the stack.
        OP_7                    = 0x57, //!< The number 7  is pushed onto the stack.
        OP_8                    = 0x58, //!< The number 8  is pushed onto the stack.
        OP_9                    = 0x59, //!< The number 9  is pushed onto the stack.
        OP_10                   = 0x5a, //!< The number 10 is pushed onto the stack.
        OP_11                   = 0x5b, //!< The number 11 is pushed onto the stack.
        OP_12                   = 0x5c, //!< The number 12 is pushed onto the stack.
        OP_13                   = 0x5d, //!< The number 13 is pushed onto the stack.
        OP_14                   = 0x5e, //!< The number 14 is pushed onto the stack.
        OP_15                   = 0x5f, //!< The number 15 is pushed onto the stack.
        OP_16                   = 0x60, //!< The number 16 is pushed onto the stack.

        // Flow control

        OP_NOP                  = 0x61, //!< Does nothing.
        OP_VER                  = 0x62, //!< Transaction is invalid unless occuring in an unexecuted OP_IF branch
        OP_IF                   = 0x63, //!< If the top stack value is not 0, the statements are executed.The top stack value is
                                        //!< removed.
        OP_NOTIF                = 0x64, //!< If the top stack value is 0, the statements are executed.The top stack value is
                                        //!< removed.
        OP_VERIF                = 0x65, //!< Transaction is invalid even when occuring in an unexecuted OP_IF branch
        OP_VERNOTIF             = 0x66, //!< Transaction is invalid even when occuring in an unexecuted OP_IF branch
        OP_ELSE                 = 0x67, //!< If the preceding OP_IF or OP_NOTIF or OP_ELSE was not executed then these statements
                                        //!< are and if the preceding OP_IF or OP_NOTIF or OP_ELSE was executed then these
                                        //!< statements are not.
        OP_ENDIF                = 0x68, //!< Ends an if / else block. All blocks must end, or the transaction is invalid. An
                                        //!< OP_ENDIF without OP_IF earlier is also invalid.
        OP_VERIFY               = 0x69, //!< Marks transaction as invalid if top stack value is not true.
        OP_RETURN               = 0x6a, //!< Marks transaction as invalid. A standard way of attaching extra data to transactions
                                        //!< is to add a zero-value output with a scriptPubKey consisting of OP_RETURN followed by
                                        //!< exactly one pushdata op.Such outputs are provably unspendable, reducing their cost to
                                        //!< the network.Currently it is usually considered non-standard(though valid) for a
                                        //!< transaction to have more than one OP_RETURN output or an OP_RETURN output with more
                                        //!< than one pushdata op.

        // Stack

        OP_TOALTSTACK           = 0x6b, //!< Puts the input onto the top of the alt stack. Removes it from the main stack.
        OP_FROMALTSTACK         = 0x6c, //!< Puts the input onto the top of the main stack. Removes it from the alt stack.
        OP_2DROP                = 0x6d, //!< Removes the top two stack items.
        OP_2DUP                 = 0x6e, //!< Duplicates the top two stack items.
        OP_3DUP                 = 0x6f, //!< Duplicates the top three stack items.
        OP_2OVER                = 0x70, //!< Copies the pair of items two spaces back in the stack to the front.
        OP_2ROT                 = 0x71, //!< The fifth and sixth items back are moved to the top of the stack.
        OP_2SWAP                = 0x72, //!< Swaps the top two pairs of items.
        OP_IFDUP                = 0x73, //!< If the top stack value is not 0, duplicate it.
        OP_DEPTH                = 0x74, //!< Puts the number of stack items onto the stack.
        OP_DROP                 = 0x75, //!< Removes the top stack item.
        OP_DUP                  = 0x76, //!< Duplicates the top stack item.
        OP_NIP                  = 0x77, //!< Removes the second-to-top stack item.
        OP_OVER                 = 0x78, //!< Copies the second-to-top stack item to the top.
        OP_PICK                 = 0x79, //!< The item n back in the stack is copied to the top.
        OP_ROLL                 = 0x7a, //!< The item n back in the stack is moved to the top.
        OP_ROT                  = 0x7b, //!< The top three items on the stack are rotated to the left.
        OP_SWAP                 = 0x7c, //!< The top two items on the stack are swapped.
        OP_TUCK                 = 0x7d, //!< The item at the top of the stack is copied and inserted before the second-to-top
                                        // item.

        // Splice

        OP_CAT                  = 0x7e, //!< Concatenates two strings.
        OP_SUBSTR               = 0x7f, //!< Returns a section of a string.
        OP_LEFT                 = 0x80, //!< Keeps only characters left of the specified point in a string.
        OP_RIGHT                = 0x81, //!< Keeps only characters right of the specified point in a string.
        OP_SIZE                 = 0x82, //!< Pushes the string length of the top element of the stack(without popping it).

        // Bitwise logic

        OP_INVERT               = 0x83, //!< Flips all of the bits in the input.
        OP_AND                  = 0x84, //!< Boolean and between each bit in the inputs.
        OP_OR                   = 0x85, //!< Boolean or between each bit in the inputs.
        OP_XOR                  = 0x86, //!< Boolean exclusive or between each bit in the inputs.
        OP_EQUAL                = 0x87, //!< Returns 1 if the inputs are exactly equal, 0 otherwise.
        OP_EQUALVERIFY          = 0x88, //!< Same as OP_EQUAL, but runs OP_VERIFY afterward.
        OP_RESERVED1            = 0x89, //!< Transaction is invalid unless occuring in an unexecuted OP_IF branch
        OP_RESERVED2            = 0x8a, //!< Transaction is invalid unless occuring in an unexecuted OP_IF branch

        // Arithmetic

        OP_1ADD                 = 0x8b, //!< 1 is added to the input.
        OP_1SUB                 = 0x8c, //!< 1 is subtracted from the input.
        OP_2MUL                 = 0x8d, //!< The input is multiplied by 2. disabled.
        OP_2DIV                 = 0x8e, //!< The input is divided by 2. disabled.
        OP_NEGATE               = 0x8f, //!< The sign of the input is flipped.
        OP_ABS                  = 0x90, //!< The input is made positive.
        OP_NOT                  = 0x91, //!< If the input is 0 or 1, it is flipped. Otherwise the output will be 0.
        OP_0NOTEQUAL            = 0x92, //!< Returns 0 if the input is 0. 1 otherwise.
        OP_ADD                  = 0x93, //!< a is added to b.
        OP_SUB                  = 0x94, //!< b is subtracted from a.
        OP_MUL                  = 0x95, //!< a is multiplied by b.
        OP_DIV                  = 0x96, //!< a is divided by b.
        OP_MOD                  = 0x97, //!< Returns the remainder after dividing a by b.
        OP_LSHIFT               = 0x98, //!< Shifts a left b bits, preserving sign.
        OP_RSHIFT               = 0x99, //!< Shifts a right b bits, preserving sign.
        OP_BOOLAND              = 0x9a, //!< If both a and b are not 0, the output is 1. Otherwise 0.
        OP_BOOLOR               = 0x9b, //!< If a or b is not 0, the output is 1. Otherwise 0.
        OP_NUMEQUAL             = 0x9c, //!< Returns 1 if the numbers are equal, 0 otherwise.
        OP_NUMEQUALVERIFY       = 0x9d, //!< Same as OP_NUMEQUAL, but runs OP_VERIFY afterward.
        OP_NUMNOTEQUAL          = 0x9e, //!< Returns 1 if the numbers are not equal, 0 otherwise.
        OP_LESSTHAN             = 0x9f, //!< Returns 1 if a is less than b, 0 otherwise.
        OP_GREATERTHAN          = 0xa0, //!< Returns 1 if a is greater than b, 0 otherwise.
        OP_LESSTHANOREQUAL      = 0xa1, //!< Returns 1 if a is less than or equal to b, 0 otherwise.
        OP_GREATERTHANOREQUAL   = 0xa2, //!< Returns 1 if a is greater than or equal to b, 0 otherwise.
        OP_MIN                  = 0xa3, //!< Returns the smaller of a and b.
        OP_MAX                  = 0xa4, //!< Returns the larger of a and b.
        OP_WITHIN               = 0xa5, //!< Returns 1 if x is within the specified range (left-inclusive), 0 otherwise.

        // Crypto

        OP_RIPEMD160            = 0xa6, //!< The input is hashed using RIPEMD-160.
        OP_SHA1                 = 0xa7, //!< The input is hashed using SHA-1.
        OP_SHA256               = 0xa8, //!< The input is hashed using SHA-256.
        OP_HASH160              = 0xa9, //!< The input is hashed twice : first with SHA-256 and then with RIPEMD-160.
        OP_HASH256              = 0xaa, //!< The input is hashed two times with SHA-256.
        OP_CODESEPARATOR        = 0xab, //!< All of the signature checking words will only match signatures to the data after the
                                        //!< most recently-executed OP_CODESEPARATOR.
        OP_CHECKSIG             = 0xac, //!< The entire transaction's outputs, inputs, and script (from the most recently-executed
                                        //!< OP_CODESEPARATOR to the end) are hashed. The signature used by OP_CHECKSIG must be a
                                        //!< valid signature for this hash and public key. If it is, 1 is returned, 0 otherwise.
        OP_CHECKSIGVERIFY       = 0xad, //!< Same as OP_CHECKSIG, but OP_VERIFY is executed afterward.
        OP_CHECKMULTISIG        = 0xae, //!< Compares the first signature against each public key until it finds an ECDSA
                                        //!< match.Starting with the subsequent public key, it compares the second signature against
                                        //!< each remaining public key until it finds an ECDSA match.The process is repeated until
                                        //!< all signatures have been checked or not enough public keys remain to produce a
                                        //!< successful result.All signatures need to match a public key.Because public keys are not
                                        //!< checked again if they fail any signature comparison, signatures must be placed in the
                                        //!< scriptSig using the same order as their corresponding public keys were placed in the
                                        //!< scriptPubKey or redeemScript.If all signatures are valid, 1 is returned, 0
                                        //!< otherwise. Due to a bug, one extra unused value is removed from the stack.
        OP_CHECKMULTISIGVERIFY  = 0xaf, //!< Same as OP_CHECKMULTISIG, but OP_VERIFY is executed afterward.
        OP_NOP1                 = 0xb0, //!< The word is ignored. Does not mark transaction as invalid.
        OP_CHECKLOCKTIMEVERIFY  = 0xb1, //!< Marks transaction as invalid if the top stack item is greater than the transaction's
                                        //!< nLockTime field, otherwise script evaluation continues as though an OP_NOP was
                                        //!< executed. Transaction is also invalid if 1. the top stack item is negative; or 2. the
                                        //!< top stack item is greater than or equal to 500000000 while the transaction's nLockTime
                                        //!< field is less than 500000000, or vice versa; or 3. the input's nSequence field is equal
                                        //!< to 0xffffffff. The precise semantics are described in BIP 0065
        OP_NOP3                 = 0xb2, //!< The word is ignored. Does not mark transaction as invalid.
        OP_NOP4                 = 0xb3, //!< The word is ignored. Does not mark transaction as invalid.
        OP_NOP5                 = 0xb4, //!< The word is ignored. Does not mark transaction as invalid.
        OP_NOP6                 = 0xb5, //!< The word is ignored. Does not mark transaction as invalid.
        OP_NOP7                 = 0xb6, //!< The word is ignored. Does not mark transaction as invalid.
        OP_NOP8                 = 0xb7, //!< The word is ignored. Does not mark transaction as invalid.
        OP_NOP9                 = 0xb8, //!< The word is ignored. Does not mark transaction as invalid.
        OP_NOP10                = 0xb9, //!< The word is ignored. Does not mark transaction as invalid.

        // 0xba-0xfc                    // Not assigned

        // 0xfd-0xff                    // Reserved for internal use

        OP_INVALID              = 0xff  //!< Denotes an invalid instruction
    };

    struct Description
    {
        int value;          // Opcode value
        char const * name;  // Name to display
        int minArgs;        // Minimum number of arguments
        bool valid;         // If false, the appearance of this opcode in a script will invalidate the script
    };

    Instruction(int op, std::vector<uint8_t> const & data = std::vector<uint8_t>(), size_t location = 0);
    Instruction(uint8_t const * & in, size_t & size, size_t location);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Returns the opcode
    int op() const { return op_; }

    //! Returns the data (if any)
    std::vector<uint8_t> const & data() const { return data_; }

    //! Returns the location of the instruction in the script
    size_t location() const { return location_; }

    //! Returns the size of the instruction in the script
    size_t size() const { return size_; }

    //! Returns true if the instruction is well-formed and its appearance does not mark the script as invalid
    bool valid() const { return valid_; }

    //! Returns information related to this instruction's opcode
    Description const & description() const { return getDescription(op_); }

    //! Returns information related to the given opcode (whether valid or not)
    static Description const & getDescription(int opcode);

private:

    int op_;                        // Opcode
    std::vector<uint8_t> data_;     // Data
    size_t location_;               // Location of the instruction
    size_t size_;                   // Size of the instruction (also offset to the next instruction)
    bool valid_;                    // If true, then the instruction is properly formed and the appearance does not
                                    // mark the transaction as invalid

    static size_t const NUMBER_OF_INSTRUCTIONS = 256;
    static Description const DESCRIPTIONS[NUMBER_OF_INSTRUCTIONS];
};

//! A list of instructions
typedef std::vector<Instruction> Program;

} // namespace Equity
