//! BIP-39 mnemonic — 12/15/18/21/24-word seed phrases.
//!
//! Validate() only requires
//! that all words are in the dictionary and the count is a multiple of 3.
//! Checksum is only enforced when the mnemonic is constructed from entropy; seed derivation works on any
//! syntactically valid sentence.

use crate::mnemonic_wordlist::ENGLISH_WORDLIST;
use crate::{EquityError, Result};
use crypto::{pbkdf2, sha256};

const BITS_PER_WORD: usize = 11;
const BYTES_PER_CHECK_BIT: usize = 4; // 32 entropy bits per checksum bit
const SEED_SIZE: usize = 64;
const PBKDF2_ROUNDS: u32 = 2048;

/// BIP-39 wordlist language. Only English is supported.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Language {
    /// English BIP-39 wordlist.
    English,
}

impl Language {
    fn dictionary(self) -> &'static [&'static str; 2048] {
        match self {
            Language::English => &ENGLISH_WORDLIST,
        }
    }
}

/// BIP-39 mnemonic phrase.
#[derive(Debug, Clone)]
pub struct Mnemonic {
    words: Vec<String>,
    valid: bool,
    language: Language,
}

impl Mnemonic {
    /// Build from an explicit word list.
    pub fn from_words(words: &[&str]) -> Self {
        let owned: Vec<String> = words.iter().map(|w| w.to_string()).collect();
        let language = determine_language(&owned);
        let valid = validate_words(&owned, language);
        Self {
            words: owned,
            valid,
            language,
        }
    }

    /// Build from a whitespace-separated mnemonic sentence.
    pub fn from_sentence(sentence: &str) -> Self {
        let words: Vec<&str> = sentence.split_whitespace().collect();
        Self::from_words(&words)
    }

    /// Derive a mnemonic from raw entropy.
    ///
    /// `entropy.len()` must be a multiple of 4 (128..=256 bits per BIP-39).
    ///
    /// # Examples
    ///
    /// ```
    /// use equity::mnemonic::{Mnemonic, Language};
    ///
    /// let m = Mnemonic::from_entropy(&[0u8; 16], Language::English).unwrap();
    /// assert!(m.sentence().starts_with("abandon abandon"));
    /// ```
    pub fn from_entropy(entropy: &[u8], language: Language) -> Result<Self> {
        if !entropy.len().is_multiple_of(BYTES_PER_CHECK_BIT)
            || entropy.len() > 256 * 8 * BYTES_PER_CHECK_BIT
        {
            return Err(EquityError("Invalid entropy length".to_string()));
        }
        let hash = sha256::sha256(entropy);
        let checksum_bytes = entropy.len().div_ceil(BYTES_PER_CHECK_BIT * 8);
        let mut bits = entropy.to_vec();
        bits.extend_from_slice(&hash[..checksum_bytes]);

        let dictionary = language.dictionary();
        let mut words: Vec<String> = Vec::new();
        let mut b: usize = 0;
        let mut d: u32 = 0;
        let mut it = bits.iter();
        loop {
            while b < BITS_PER_WORD {
                let Some(byte) = it.next() else { break };
                d = (d << 8) | (*byte as u32);
                b += 8;
            }
            if b < BITS_PER_WORD {
                break;
            }
            let k = ((d >> (b - BITS_PER_WORD)) & ((1u32 << BITS_PER_WORD) - 1)) as usize;
            words.push(dictionary[k].to_string());
            b -= BITS_PER_WORD;
        }

        let valid = validate_words(&words, language);
        debug_assert!(valid);
        Ok(Self {
            words,
            valid,
            language,
        })
    }

    /// True if every word is in the dictionary and the count is a multiple of 3.
    pub fn is_valid(&self) -> bool {
        self.valid
    }

    /// Borrowed word list.
    pub fn words(&self) -> &[String] {
        &self.words
    }

    /// Space-joined sentence form. Empty string when invalid.
    pub fn sentence(&self) -> String {
        if !self.valid {
            return String::new();
        }
        self.words.join(" ")
    }

    /// Mnemonic wordlist language.
    pub fn language(&self) -> Language {
        self.language
    }

    /// Recover the underlying entropy by stripping the checksum bits. Empty when invalid.
    pub fn entropy(&self) -> Vec<u8> {
        if !self.valid {
            return Vec::new();
        }
        let mut e = self.checked_entropy();
        let entropy_bytes =
            self.words.len() * BITS_PER_WORD / (BYTES_PER_CHECK_BIT * 8 + 1) * BYTES_PER_CHECK_BIT;
        e.truncate(entropy_bytes);
        e
    }

    /// Derive a 64-byte BIP-39 seed from the mnemonic and optional `password` (passphrase).
    ///
    /// Internally runs PBKDF2-HMAC-SHA-512 with 2048 iterations over the
    /// sentence and the salt `"mnemonic" + password`.
    ///
    /// # Examples
    ///
    /// ```
    /// use equity::mnemonic::{Mnemonic, Language};
    ///
    /// let m = Mnemonic::from_entropy(&[0u8; 16], Language::English).unwrap();
    /// let seed = m.seed("TREZOR");
    /// assert_eq!(seed.len(), 64);
    /// ```
    pub fn seed(&self, password: &str) -> Vec<u8> {
        if !self.valid {
            return Vec::new();
        }
        let sentence = self.sentence();
        let salt = format!("mnemonic{password}");
        pbkdf2::pbkdf2_hmac_sha512(
            sentence.as_bytes(),
            salt.as_bytes(),
            PBKDF2_ROUNDS,
            SEED_SIZE,
        )
        .unwrap_or_default()
    }

    /// Dictionary words starting with `partial`. Pass `max = 0` for "unlimited".
    pub fn suggestions(&self, partial: &str, max: usize) -> Vec<String> {
        let dictionary = self.language.dictionary();
        let mut out = Vec::new();
        for w in dictionary.iter() {
            if w.starts_with(partial) {
                out.push((*w).to_string());
                if max > 0 && out.len() >= max {
                    break;
                }
            }
        }
        out
    }

    fn checked_entropy(&self) -> Vec<u8> {
        let dictionary = self.language.dictionary();
        let mut e: Vec<u8> = Vec::with_capacity((self.words.len() * BITS_PER_WORD).div_ceil(8));
        let mut b: i32 = 0;
        let mut d: u32 = 0;
        for w in &self.words {
            let offset = dictionary
                .iter()
                .position(|x| *x == w.as_str())
                .expect("word in dictionary") as u32;
            d = (d << BITS_PER_WORD) | offset;
            b += BITS_PER_WORD as i32;
            while b > 8 {
                e.push(((d >> (b - 8)) & 0xff) as u8);
                b -= 8;
            }
        }
        if b > 0 {
            e.push(((d << (8 - b)) & 0xff) as u8);
        }
        e
    }
}

fn validate_words(words: &[String], language: Language) -> bool {
    if words.is_empty() {
        return false;
    }
    let words_per_group = (BYTES_PER_CHECK_BIT * 8 + 1) / BITS_PER_WORD; // = 3
    if !words.len().is_multiple_of(words_per_group) {
        return false;
    }
    are_found(words, language)
}

fn are_found(words: &[String], language: Language) -> bool {
    let dictionary = language.dictionary();
    words.iter().all(|w| dictionary.contains(&w.as_str()))
}

fn determine_language(_words: &[String]) -> Language {
    Language::English
}

/// Generate a random mnemonic with `strength` entropy bits (one of 128, 160,
/// 192, 224, 256).
///
/// # Examples
///
/// ```
/// use equity::mnemonic::{generate_mnemonic, validate_mnemonic};
///
/// let phrase = generate_mnemonic(128).unwrap();
/// assert!(validate_mnemonic(&phrase));
/// ```
pub fn generate_mnemonic(strength: u32) -> std::result::Result<String, String> {
    if !strength.is_multiple_of(8) {
        return Err("strength must be byte-aligned".into());
    }
    let bytes = (strength / 8) as usize;
    if !(16..=32).contains(&bytes) || !bytes.is_multiple_of(4) {
        return Err(format!("invalid strength: {strength}"));
    }
    let entropy = crypto::random::get_bytes(bytes);
    let m = Mnemonic::from_entropy(&entropy, Language::English).map_err(|e| e.0)?;
    Ok(m.sentence())
}

/// True if every whitespace-separated word in `sentence` is in the dictionary
/// and the word count is a multiple of three.
///
/// Note: the BIP-39 checksum is *not* enforced here — only structural
/// validity. Checksums are verified when a mnemonic is constructed via
/// [`Mnemonic::from_entropy`].
pub fn validate_mnemonic(sentence: &str) -> bool {
    Mnemonic::from_sentence(sentence).is_valid()
}

/// Convenience: derive the BIP-39 seed for `sentence` with the given passphrase.
///
/// # Examples
///
/// ```
/// use equity::mnemonic::mnemonic_to_seed;
///
/// let seed = mnemonic_to_seed(
///     "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
///     "TREZOR",
/// ).unwrap();
/// assert_eq!(seed.len(), 64);
/// ```
pub fn mnemonic_to_seed(sentence: &str, passphrase: &str) -> std::result::Result<Vec<u8>, String> {
    let m = Mnemonic::from_sentence(sentence);
    if !m.is_valid() {
        return Err("invalid mnemonic".into());
    }
    Ok(m.seed(passphrase))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_mnemonic_generation_round_trip() {
        let phrase = generate_mnemonic(128).unwrap();
        assert!(!phrase.is_empty());
        assert!(validate_mnemonic(&phrase));
        let seed = mnemonic_to_seed(&phrase, "").unwrap();
        assert_eq!(seed.len(), SEED_SIZE);
    }

    #[test]
    fn test_invalid_mnemonic_rejected() {
        assert!(!validate_mnemonic("invalid mnemonic phrase"));
        assert!(!validate_mnemonic(""));
    }

    #[test]
    fn test_known_bip39_vector() {
        // BIP-39 test vector: all-zero 128-bit entropy.
        let entropy = [0u8; 16];
        let m = Mnemonic::from_entropy(&entropy, Language::English).unwrap();
        assert_eq!(
            m.sentence(),
            "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about"
        );
        let seed = m.seed("TREZOR");
        // Expected seed for "abandon..." / "TREZOR" per BIP-39 test vectors.
        let expected_hex = "c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e53495531f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04";
        assert_eq!(hex::encode(seed), expected_hex);
    }

    #[test]
    fn test_entropy_round_trip() {
        let entropy = [0xABu8; 16];
        let m = Mnemonic::from_entropy(&entropy, Language::English).unwrap();
        assert_eq!(m.entropy(), entropy.to_vec());
    }

    #[test]
    fn test_suggestions_prefix() {
        let m = Mnemonic::from_sentence(
            "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
        );
        let s = m.suggestions("aban", 0);
        assert_eq!(s, vec!["abandon".to_string()]);
    }

    #[test]
    fn test_word_count_not_multiple_of_three_rejected() {
        // Two dictionary words — invalid count.
        assert!(!validate_mnemonic("abandon abandon"));
    }

    #[test]
    fn test_unknown_word_rejected() {
        assert!(!validate_mnemonic(
            "abandon abandon notaword abandon abandon abandon abandon abandon abandon abandon abandon about"
        ));
    }

    #[test]
    fn test_generate_strengths() {
        for strength in [128u32, 160, 192, 224, 256] {
            let phrase = generate_mnemonic(strength).unwrap();
            let words: Vec<&str> = phrase.split_whitespace().collect();
            let expected_words = (strength as usize + strength as usize / 32) / 11;
            assert_eq!(words.len(), expected_words, "strength {strength}");
            assert!(validate_mnemonic(&phrase));
        }
    }

    #[test]
    fn test_generate_invalid_strength() {
        assert!(generate_mnemonic(64).is_err());
        assert!(generate_mnemonic(100).is_err()); // not byte-aligned
        assert!(generate_mnemonic(140).is_err()); // not %4 bytes
        assert!(generate_mnemonic(512).is_err()); // > 32 bytes
    }

    #[test]
    fn test_known_bip39_seed_with_passphrase() {
        // BIP-39: 24-word "abandon * 23 art" with passphrase "TREZOR".
        let entropy = [0u8; 32];
        let m = Mnemonic::from_entropy(&entropy, Language::English).unwrap();
        assert!(m.sentence().ends_with(" art"));
        assert_eq!(m.words().len(), 24);
        let seed = m.seed("TREZOR");
        let expected = "bda85446c68413707090a52022edd26a1c9462295029f2e60cd7c4f2bbd3097170af7a4d73245cafa9c3cca8d561a7c3de6f5d4a10be8ed2a5e608d68f92fcc8";
        assert_eq!(hex::encode(seed), expected);
    }

    #[test]
    fn test_invalid_entropy_length() {
        // Not multiple of 4 bytes.
        assert!(Mnemonic::from_entropy(&[0u8; 15], Language::English).is_err());
        assert!(Mnemonic::from_entropy(&[0u8; 17], Language::English).is_err());
    }

    #[test]
    fn test_mnemonic_to_seed_invalid_rejected() {
        assert!(mnemonic_to_seed("nope nope nope", "").is_err());
    }
}
