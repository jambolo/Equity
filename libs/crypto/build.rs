fn main() {
    cxx_build::bridge("src/lib.rs")
        .file("../../crypto/Ecc.cpp")
        .file("../../crypto/Hmac.cpp")
        .file("../../crypto/Pbkdf2.cpp")
        .file("../../crypto/Random.cpp")
        .file("../../crypto/Ripemd.cpp")
        .file("../../crypto/Sha1.cpp")
        .file("../../crypto/Sha256.cpp")
        .file("../../crypto/Sha512.cpp")
        .file("src/crypto_wrapper.cpp")
        .include("../../crypto")
        .include("../../include")
        .include("../../utility")
        .include("../../")
        .include("src")
        .include("/usr/local/include")
        .include("C:/Users/John/Projects/3rdParty/wolfssl/include")  // Try include subdirectory
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wno-unused-parameter")
        .compile("crypto");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/crypto_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/crypto_wrapper.h");
    println!("cargo:rerun-if-changed=../../crypto/");
    println!("cargo:rustc-link-search=native=/usr/local/lib");
    println!("cargo:rustc-link-search=native=C:/Users/John/Projects/3rdParty/wolfssl/lib");  // Add WolfSSL lib search path
    println!("cargo:rustc-link-search=native=C:/Users/John/Projects/3rdParty/wolfssl");  // Also try root directory
    println!("cargo:rustc-link-lib=wolfssl");
    
    // Link Windows CryptoAPI library that wolfssl requires
    #[cfg(target_os = "windows")]
    {
        println!("cargo:rustc-link-lib=advapi32");
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=ws2_32");
    }
}
