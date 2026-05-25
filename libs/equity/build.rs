fn main() {
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/equity_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/equity_wrapper.h");
    println!("cargo:rerun-if-changed=../../equity/");

    if std::env::var("CARGO_FEATURE_CXX_BRIDGE").is_err() {
        return;
    }

    if !std::path::Path::new("../../equity").exists() {
        println!("cargo:warning=Equity C++ sources not found, using Rust-only implementation");
        return;
    }

    let nlohmann = std::env::var("NLOHMANN_ROOT").unwrap_or_else(|_| {
        "C:/Users/John/Projects/3rdParty/nlohmann_json/include".to_string()
    });
    let wolfssl = std::env::var("WOLFSSL_ROOT")
        .unwrap_or_else(|_| "C:/Users/John/Projects/3rdParty/wolfssl".to_string());

    println!("cargo:warning=Building Equity bridge from C++ sources");

    cxx_build::bridge("src/lib.rs")
        .file("../../equity/Address.cpp")
        .file("../../equity/Base58.cpp")
        .file("../../equity/Base58Check.cpp")
        .file("../../equity/Block.cpp")
        .file("../../equity/Configuration.cpp")
        .file("../../equity/Instruction.cpp")
        .file("../../equity/MerkleTree.cpp")
        .file("../../equity/Mnemonic.cpp")
        .file("../../equity/PrivateKey.cpp")
        .file("../../equity/PublicKey.cpp")
        .file("../../equity/Script.cpp")
        .file("../../equity/ScriptEngine.cpp")
        .file("../../equity/Target.cpp")
        .file("../../equity/Transaction.cpp")
        .file("../../equity/Txid.cpp")
        .file("../../equity/Validator.cpp")
        .file("../../equity/Wallet.cpp")
        // Crypto C++ sources (equity C++ code calls into these directly)
        .file("../../crypto/Ecc.cpp")
        .file("../../crypto/Ripemd.cpp")
        .file("../../crypto/Sha256.cpp")
        .file("../../crypto/Sha512.cpp")
        .file("../../crypto/Sha1.cpp")
        .file("../../crypto/Hmac.cpp")
        .file("../../crypto/Pbkdf2.cpp")
        .file("../../crypto/Random.cpp")
        // Utility C++ sources (equity C++ code calls into these directly)
        .file("../../utility/Utility.cpp")
        .file("../../utility/Endian.cpp")
        .file("src/equity_wrapper.cpp")
        .include("../../equity")
        .include("../../include")
        .include("../../include/equity")
        .include("../../crypto")
        .include("../../utility")
        .include("../../network")
        .include("../../p2p")
        .include("../../")
        .include("src")
        .include(&nlohmann)
        .include(format!("{wolfssl}/include"))
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wno-unused-parameter")
        .flag_if_supported("/EHsc")
        .compile("equity_bridge");

    println!("cargo:rustc-link-search=native={wolfssl}/lib");
    println!("cargo:rustc-link-search=native={wolfssl}");
    println!("cargo:rustc-link-lib=wolfssl");
    if cfg!(target_os = "windows") {
        println!("cargo:rustc-link-lib=advapi32");
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=ws2_32");
    }
}
