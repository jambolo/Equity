//
//  Mnemonic.h
//  Equity
//
//  Created by John Bolton on 6/5/16.
//
//

#ifndef Mnemonic_h
#define Mnemonic_h

#include <vector>
#include <string>
#include <cstdint>

//! Mnemonic Generator.
//!
//! This class implement BIP-39.
//! @sa https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki

class Mnemonic
{
public:
    
    using WordList = std::vector<std::string>;
    
    // Constructor
    //!
    //! @param  words   mnemonic
    //! @note   If the mnemonic is not valid, invalid size or checksum, then isValid() will return false.
    explicit Mnemonic(WordList const & words);
    
    // Constructor
    //!
    //! @param  bits    binary source for the mnemonic
    //! @note   The binary data does not include a checksum
    //! @note   The size of the data must be a multiple of 4. If not, isValid() will return false.
    explicit Mnemonic(std::vector<uint8_t> const & bits) : Mnemonic(bits.data(), bits.size()) {}

    // Constructor
    //!
    //! @param  bits    binary source for the mnemonic
    //! @param  size    size of binary data
    //! @note   The binary data does not include a checksum
    //! @note   The size of the data must be a multiple of 4. If not, isValid() will return false.
    Mnemonic(uint8_t const * bits, size_t size);
    
    //! Returns the mnemonic
    WordList words() const { return words_; }
    
    //! Returns true if the word list is valid
    bool isValid() const { return valid_; };
    
    //! Returns a list of the possible words given a partial word
    static WordList hint(std::string const & word, size_t max = 0);
    
private:
    
    bool verify() const;
    
    WordList words_;
    bool valid_;
};

#endif /* Mnemonic_h */
