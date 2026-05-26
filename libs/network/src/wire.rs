//! Wire envelope: header + payload framing for Bitcoin P2P messages.

use crate::header::{HEADER_SIZE, Header, payload_checksum};
use crate::messages::Message;
use anyhow::{Result, bail};

/// A framed message: header (24 bytes) followed by the serialized payload.
#[derive(Debug, Clone)]
pub struct WireMessage {
    /// Decoded message header.
    pub header: Header,
    /// Raw payload bytes following the header.
    pub payload: Vec<u8>,
}

impl WireMessage {
    /// Encode a typed `Message` into a framed wire packet with the given magic.
    pub fn encode(message: &Message, magic: u32) -> Result<Self> {
        let payload = message.payload_bytes();
        let header = Header::new(magic, message.command(), &payload)?;
        Ok(Self { header, payload })
    }

    /// Concatenated header + payload.
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(HEADER_SIZE + self.payload.len());
        self.header.serialize(&mut out);
        out.extend_from_slice(&self.payload);
        out
    }

    /// Parse a complete frame (header + payload) and verify checksum.
    pub fn from_bytes(data: &[u8]) -> Result<Self> {
        let mut s = data;
        let header = Header::deserialize(&mut s)?;
        if s.len() < header.length as usize {
            bail!(
                "wire payload truncated (need {}, have {})",
                header.length,
                s.len()
            );
        }
        let payload = s[..header.length as usize].to_vec();
        if payload_checksum(&payload) != header.checksum {
            bail!("wire message checksum mismatch");
        }
        Ok(Self { header, payload })
    }

    /// Decode the payload into a typed `Message`.
    pub fn decode(&self) -> Result<Message> {
        Message::deserialize_payload(self.header.command_str(), &self.payload)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::header::MAGIC_MAIN;
    use crate::messages::PingMessage;

    #[test]
    fn ping_envelope_round_trip() {
        let msg = Message::Ping(PingMessage { nonce: 42 });
        let wire = WireMessage::encode(&msg, MAGIC_MAIN).unwrap();
        let bytes = wire.to_bytes();
        assert_eq!(bytes.len(), HEADER_SIZE + 8);
        let decoded = WireMessage::from_bytes(&bytes).unwrap();
        let dmsg = decoded.decode().unwrap();
        match dmsg {
            Message::Ping(p) => assert_eq!(p.nonce, 42),
            _ => panic!("wrong type"),
        }
    }

    #[test]
    fn corrupted_payload_fails_checksum() {
        let msg = Message::Ping(PingMessage { nonce: 1 });
        let mut bytes = WireMessage::encode(&msg, MAGIC_MAIN).unwrap().to_bytes();
        let last = bytes.len() - 1;
        bytes[last] ^= 0xFF;
        assert!(WireMessage::from_bytes(&bytes).is_err());
    }

    #[test]
    fn verack_round_trip() {
        let wire = WireMessage::encode(&Message::Verack, MAGIC_MAIN).unwrap();
        let bytes = wire.to_bytes();
        assert_eq!(bytes.len(), HEADER_SIZE);
        let decoded = WireMessage::from_bytes(&bytes).unwrap();
        assert!(matches!(decoded.decode().unwrap(), Message::Verack));
    }

    #[test]
    fn truncated_header_rejected() {
        assert!(WireMessage::from_bytes(&[0u8; HEADER_SIZE - 1]).is_err());
    }

    #[test]
    fn truncated_payload_rejected() {
        let msg = Message::Ping(PingMessage { nonce: 9 });
        let bytes = WireMessage::encode(&msg, MAGIC_MAIN).unwrap().to_bytes();
        // Drop last payload byte.
        assert!(WireMessage::from_bytes(&bytes[..bytes.len() - 1]).is_err());
    }

    #[test]
    fn all_empty_payload_variants_round_trip() {
        for m in [
            Message::Verack,
            Message::GetAddr,
            Message::FilterClear,
            Message::SendHeaders,
            Message::Mempool,
        ] {
            let cmd = m.command();
            let bytes = WireMessage::encode(&m, MAGIC_MAIN).unwrap().to_bytes();
            let decoded = WireMessage::from_bytes(&bytes).unwrap();
            assert_eq!(decoded.header.command_str(), cmd);
            assert!(decoded.payload.is_empty());
        }
    }
}
