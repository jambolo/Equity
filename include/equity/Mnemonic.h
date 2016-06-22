//
//  Mnemonic.h
//  Equity
//
//  Created by John Bolton on 6/5/16.
//
//

#ifndef Mnemonic_h
#define Mnemonic_h

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{

//! Mnemonic Generator.
//!
//! This class implements BIP-39.
//! @sa https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki

class Mnemonic
{
public:

    //! A list of words.
    using WordList = std::vector<std::string>;

    // Constructor
    //!
    //! @param  words   mnemonic sentence
    //! @note   If the mnemonic is not valid, invalid size or checksum, then isValid() will return false.
    explicit Mnemonic(WordList const & words);

    // Constructor
    //!
    //! @param  entropy     entropy source
    //! @note   The entropy data does not include a checksum
    //! @note   The size of the entropy must be a multiple of 4. If not, isValid() will return false.
    explicit Mnemonic(std::vector<uint8_t> const & entropy) : Mnemonic(entropy.data(), entropy.size()) {}

    // Constructor
    //!
    //! @param  entropy     entropy source for the mnemonic
    //! @param  size        size of entropy data
    //! @note   The entropy data does not include a checksum
    //! @note   The size of the entropy must be a multiple of 4. If not, isValid() will return false.
    Mnemonic(uint8_t const * entropy, size_t size);

    //! Returns the 512-bit seed computed from the mnemonic sentence.
    //!
    //! @param  password    password used to generate the seed (default: "")
    //! @return     A 512-bit seed value
    std::vector<uint8_t> seed(char const * password = "") const;

    //! Returns the original entropy data used to generate the mnemonic.
    std::vector<uint8_t> entropy() const;

    //! Returns the mnemonic sentence.
    //!
    //! The sentence is simply a string containing the words in order separated by a spaces.
    std::string sentence() const;

    //! Returns the mnemonic sentence
    WordList words() const { return words_; }

    //! Returns true if the Mnemonic is valid
    bool isValid() const { return valid_; }

    //! Returns a list of the possible words given a partial word
    //! hint.
    //!
    //! @param  word    partial word to test
    //! @param  max     maximum number of suggestions to return
    //!
    //! @return     list of suggestions
    static WordList suggestions(std::string const & word, size_t max = 0);

private:

    static size_t const BITS_PER_WORD = 11;
    static size_t const BYTES_PER_CHECK_BIT = 4; // 32 bits
    static size_t const SEED_SIZE = 512 / 8;
    static int const PBKDF2_ROUNDS = 2048;

    using Dictionary = std::array<char const *, 1 << BITS_PER_WORD>;

    bool validate() const;
    std::vector<uint8_t> checkedEntropy() const;
    static Dictionary::const_iterator find(std::string const & word);

    WordList words_;
    bool valid_;

    static Dictionary const DICTIONARY;
};

} // namespace Equity

#endif /* Mnemonic_h */
