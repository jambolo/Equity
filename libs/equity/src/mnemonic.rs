//! BIP-39 mnemonic — 12/15/18/21/24-word seed phrases.
//!
//! Layout follows the original C++ implementation: validate() only requires
//! that all words are in the dictionary and the count is a multiple of 3.
//! Checksum is only enforced when the mnemonic is constructed from entropy
//! (matching C++'s asymmetric behaviour); seed derivation works on any
//! syntactically valid sentence.

use crate::mnemonic_wordlist::ENGLISH_WORDLIST;
use crate::{EquityError, Result};
use crypto::{pbkdf2, sha256};

const BITS_PER_WORD: usize = 11;
const BYTES_PER_CHECK_BIT: usize = 4; // 32 entropy bits per checksum bit
const SEED_SIZE: usize = 64;
const PBKDF2_ROUNDS: i32 = 2048;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Language {
    English,
}

impl Language {
    fn dictionary(self) -> &'static [&'static str; 2048] {
        match self {
            Language::English => &ENGLISH_WORDLIST,
        }
    }
}

#[derive(Debug, Clone)]
pub struct Mnemonic {
    words: Vec<String>,
    valid: bool,
    language: Language,
}

impl Mnemonic {
    pub fn from_words(words: &[&str]) -> Self {
        let owned: Vec<String> = words.iter().map(|w| w.to_string()).collect();
        let language = determine_language(&owned);
        let valid = validate_words(&owned, language);
        Self { words: owned, valid, language }
    }

    pub fn from_sentence(sentence: &str) -> Self {
        let words: Vec<&str> = sentence.split_whitespace().collect();
        Self::from_words(&words)
    }

    pub fn from_entropy(entropy: &[u8], language: Language) -> Result<Self> {
        if entropy.len() % BYTES_PER_CHECK_BIT != 0 || entropy.len() > 256 * 8 * BYTES_PER_CHECK_BIT
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
        Ok(Self { words, valid, language })
    }

    pub fn is_valid(&self) -> bool {
        self.valid
    }

    pub fn words(&self) -> &[String] {
        &self.words
    }

    pub fn sentence(&self) -> String {
        if !self.valid {
            return String::new();
        }
        self.words.join(" ")
    }

    pub fn language(&self) -> Language {
        self.language
    }

    pub fn entropy(&self) -> Vec<u8> {
        if !self.valid {
            return Vec::new();
        }
        let mut e = self.checked_entropy();
        let entropy_bytes = self.words.len() * BITS_PER_WORD / (BYTES_PER_CHECK_BIT * 8 + 1)
            * BYTES_PER_CHECK_BIT;
        e.truncate(entropy_bytes);
        e
    }

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
    if words.len() % words_per_group != 0 {
        return false;
    }
    are_found(words, language)
}

fn are_found(words: &[String], language: Language) -> bool {
    let dictionary = language.dictionary();
    words.iter().all(|w| dictionary.iter().any(|d| *d == w.as_str()))
}

fn determine_language(words: &[String]) -> Language {
    if are_found(words, Language::English) {
        Language::English
    } else {
        Language::English // sole supported language; fall back as in C++
    }
}

// Convenience wrappers for the prior wrapper-style API.
pub fn generate_mnemonic(strength: u32) -> std::result::Result<String, String> {
    if strength % 8 != 0 {
        return Err("strength must be byte-aligned".into());
    }
    let bytes = (strength / 8) as usize;
    if !(16..=32).contains(&bytes) || bytes % 4 != 0 {
        return Err(format!("invalid strength: {strength}"));
    }
    let entropy = crypto::random::get_bytes(bytes);
    let m = Mnemonic::from_entropy(&entropy, Language::English).map_err(|e| e.0)?;
    Ok(m.sentence())
}

pub fn validate_mnemonic(sentence: &str) -> bool {
    Mnemonic::from_sentence(sentence).is_valid()
}

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
}
