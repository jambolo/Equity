use crate::ffi;

/// Mnemonic generation and validation utilities
pub struct Mnemonic;

impl Mnemonic {
    /// Generate a mnemonic phrase with the specified bit strength
    pub fn generate(strength: u32) -> Result<String, String> {
        let mut output = String::new();
        if ffi::mnemonicGenerate(strength, &mut output) {
            Ok(output)
        } else {
            Err(format!("Failed to generate mnemonic with strength {}", strength))
        }
    }

    /// Validate a mnemonic phrase
    pub fn validate(mnemonic: &str) -> bool {
        ffi::mnemonicValidate(mnemonic)
    }

    /// Convert a mnemonic phrase to a seed
    pub fn to_seed(mnemonic: &str, passphrase: &str) -> Result<Vec<u8>, String> {
        let mut seed = Vec::new();
        if ffi::mnemonicToSeed(mnemonic, passphrase, &mut seed) {
            Ok(seed)
        } else {
            Err("Failed to convert mnemonic to seed".to_string())
        }
    }
}

// Standalone function exports for convenience
pub fn generate_mnemonic(strength: u32) -> Result<String, String> {
    Mnemonic::generate(strength)
}

pub fn validate_mnemonic(mnemonic: &str) -> bool {
    Mnemonic::validate(mnemonic)
}

pub fn mnemonic_to_seed(mnemonic: &str, passphrase: &str) -> Result<Vec<u8>, String> {
    Mnemonic::to_seed(mnemonic, passphrase)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_mnemonic_generation() {
        // Test with 128-bit strength (should work with our test implementation)
        let result = generate_mnemonic(128);
        assert!(result.is_ok());
        
        let mnemonic = result.unwrap();
        assert!(!mnemonic.is_empty());
        
        // Test validation
        assert!(validate_mnemonic(&mnemonic));
        
        // Test seed generation
        let seed_result = mnemonic_to_seed(&mnemonic, "");
        assert!(seed_result.is_ok());
        
        let seed = seed_result.unwrap();
        assert_eq!(seed.len(), 64); // Expected seed length
    }

    #[test]
    fn test_invalid_mnemonic() {
        assert!(!validate_mnemonic("invalid mnemonic phrase"));
    }
}