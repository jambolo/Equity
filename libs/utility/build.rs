fn main() {
    cxx_build::bridge("src/lib.rs")
        // Utility core files
        .file("../../utility/Endian.cpp")
        .file("../../utility/MerkleTree.cpp")
        .file("../../utility/Utility.cpp")
        
        // Crypto files (required by MerkleTree)
        .file("../../crypto/Sha256.cpp")
        .file("../../crypto/Sha512.cpp")
        .file("../../crypto/Sha1.cpp")
        .file("../../crypto/Ripemd.cpp")
        .file("../../crypto/Hmac.cpp")
        .file("../../crypto/Pbkdf2.cpp")
        .file("../../crypto/Ecc.cpp")
        .file("../../crypto/Random.cpp")
        
        // Utility wrapper
        .file("src/utility_wrapper.cpp")
        
        // Include paths
        .include("../../utility")
        .include("../../include")
        .include("../../crypto")
        .include("../../p2p")
        .include("../../")
        .include("src")
        .include("/usr/local/include")
        .include("/usr/include")
        
        // Compiler flags
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wno-unused-parameter")
        .compile("utility");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/utility_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/utility_wrapper.h");
    println!("cargo:rerun-if-changed=../../utility/");
    println!("cargo:rustc-link-search=native=/usr/local/lib");
}
