//! Bitcoin Pay-to-Public-Key-Hash address — 20-byte RIPEMD-160(SHA-256(pubkey)).

use crate::public_key::PublicKey;
use crate::{EquityError, Network, Result, base58_check, configuration::Configuration};
use crypto::{ripemd, sha256};

pub const ADDRESS_SIZE: usize = ripemd::RIPEMD160_HASH_SIZE; // 20

#[derive(Clone, PartialEq, Eq)]
pub struct Address {
    value: [u8; ADDRESS_SIZE],
    valid: bool,
}

impl Address {
    pub fn from_string(s: &str) -> Result<Self> {
        let (decoded, _version) = base58_check::decode(s)?;
        Self::from_data(&decoded)
    }

    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() != ADDRESS_SIZE {
            return Err(EquityError(format!(
                "Address data must be {ADDRESS_SIZE} bytes"
            )));
        }
        let mut value = [0u8; ADDRESS_SIZE];
        value.copy_from_slice(data);
        Ok(Self { value, valid: true })
    }

    pub fn from_public_key(public_key: &PublicKey) -> Result<Self> {
        if !public_key.is_valid() {
            return Err(EquityError("Invalid public key".to_string()));
        }
        let sha = sha256::sha256(public_key.as_bytes());
        let value = ripemd::ripemd160(&sha);
        Ok(Self { value, valid: true })
    }

    pub fn from_public_key_bytes(pubkey_data: &[u8]) -> Result<Self> {
        let pk = PublicKey::from_data(pubkey_data)?;
        Self::from_public_key(&pk)
    }

    pub fn to_string(&self, network: Network) -> String {
        let version = match network {
            Network::Mainnet => Configuration::ADDRESS_VERSION as u32,
            Network::Testnet | Network::Regtest => 0x6F,
        };
        base58_check::encode(&self.value, version)
    }

    pub fn value(&self) -> Vec<u8> {
        self.value.to_vec()
    }

    pub fn as_bytes(&self) -> &[u8; ADDRESS_SIZE] {
        &self.value
    }

    pub fn is_valid(&self) -> bool {
        self.valid
    }
}

impl std::fmt::Display for Address {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_string(Network::Mainnet))
    }
}

impl std::fmt::Debug for Address {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Address")
            .field("value", &hex::encode(self.value))
            .field("valid", &self.valid)
            .finish()
    }
}

impl std::str::FromStr for Address {
    type Err = EquityError;

    fn from_str(s: &str) -> Result<Self> {
        Self::from_string(s)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_address_validation_rejects_garbage() {
        assert!(Address::from_string("invalid").is_err());
    }

    #[test]
    fn test_address_roundtrip_known_vector() {
        // 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa is the satoshi genesis-coinbase address.
        let s = "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa";
        let addr = Address::from_string(s).unwrap();
        assert_eq!(addr.to_string(Network::Mainnet), s);
    }

    #[test]
    fn test_address_from_public_key_bytes_matches_hash160() {
        let sk_bytes = [3u8; 32];
        let pk = PublicKey::from_private_key_bytes(&sk_bytes).unwrap();
        let addr = Address::from_public_key(&pk).unwrap();
        let expected = ripemd::ripemd160(&sha256::sha256(pk.as_bytes()));
        assert_eq!(addr.as_bytes(), &expected);
    }

    #[test]
    fn test_address_display_uses_mainnet_version() {
        let addr = Address::from_data(&[0u8; ADDRESS_SIZE]).unwrap();
        assert!(addr.to_string(Network::Mainnet).starts_with('1'));
    }
}
