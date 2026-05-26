//! BIP-39 test vectors from https://github.com/trezor/python-mnemonic/blob/master/vectors.json
//! Each entry: (entropy_hex, mnemonic, seed_hex). Passphrase is "TREZOR".

use equity::mnemonic::{Language, Mnemonic};

struct Vector {
    entropy: &'static str,
    mnemonic: &'static str,
    seed: &'static str,
}

const VECTORS: &[Vector] = &[
    Vector {
        entropy: "00000000000000000000000000000000",
        mnemonic: "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
        seed: "c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e53495531f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04",
    },
    Vector {
        entropy: "7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f",
        mnemonic: "legal winner thank year wave sausage worth useful legal winner thank yellow",
        seed: "2e8905819b8723fe2c1d161860e5ee1830318dbf49a83bd451cfb8440c28bd6fa457fe1296106559a3c80937a1c1069be3a3a5bd381ee6260e8d9739fce1f607",
    },
    Vector {
        entropy: "80808080808080808080808080808080",
        mnemonic: "letter advice cage absurd amount doctor acoustic avoid letter advice cage above",
        seed: "d71de856f81a8acc65e6fc851a38d4d7ec216fd0796d0a6827a3ad6ed5511a30fa280f12eb2e47ed2ac03b5c462a0358d18d69fe4f985ec81778c1b370b652a8",
    },
    Vector {
        entropy: "ffffffffffffffffffffffffffffffff",
        mnemonic: "zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo zoo wrong",
        seed: "ac27495480225222079d7be181583751e86f571027b0497b5b5d11218e0a8a13332572917f0f8e5a589620c6f15b11c61dee327651a14c34e18231052e48c069",
    },
    Vector {
        entropy: "000000000000000000000000000000000000000000000000",
        mnemonic: "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon agent",
        seed: "035895f2f481b1b0f01fcf8c289c794660b289981a78f8106447707fdd9666ca06da5a9a565181599b79f53b844d8a71dd9f439c52a3d7b3e8a79c906ac845fa",
    },
    Vector {
        entropy: "0000000000000000000000000000000000000000000000000000000000000000",
        mnemonic: "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon art",
        seed: "bda85446c68413707090a52022edd26a1c9462295029f2e60cd7c4f2bbd3097170af7a4d73245cafa9c3cca8d561a7c3de6f5d4a10be8ed2a5e608d68f92fcc8",
    },
    Vector {
        entropy: "8080808080808080808080808080808080808080808080808080808080808080",
        mnemonic: "letter advice cage absurd amount doctor acoustic avoid letter advice cage absurd amount doctor acoustic avoid letter advice cage absurd amount doctor acoustic bless",
        seed: "c0c519bd0e91a2ed54357d9d1ebef6f5af218a153624cf4f2da911a0ed8f7a09e2ef61af0aca007096df430022f7a2b6fb91661a9589097069720d015e4e982f",
    },
];

#[test]
fn bip39_known_vectors() {
    for v in VECTORS {
        let entropy = hex::decode(v.entropy).unwrap();
        let m = Mnemonic::from_entropy(&entropy, Language::English).unwrap();
        assert_eq!(m.sentence(), v.mnemonic, "entropy {}", v.entropy);
        assert!(m.is_valid());

        let seed = m.seed("TREZOR");
        assert_eq!(hex::encode(seed), v.seed, "seed for {}", v.mnemonic);

        // Reconstruct from sentence and confirm same entropy + seed.
        let parsed = Mnemonic::from_sentence(v.mnemonic);
        assert!(parsed.is_valid());
        assert_eq!(parsed.entropy(), entropy);
        assert_eq!(hex::encode(parsed.seed("TREZOR")), v.seed);
    }
}
