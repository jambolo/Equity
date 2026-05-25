fn main() {
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/crypto_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/crypto_wrapper.h");
    println!("cargo:rerun-if-changed=../../crypto/");

    if std::env::var("CARGO_FEATURE_CXX_BRIDGE").is_err() {
        return;
    }

    let wolfssl = std::env::var("WOLFSSL_ROOT")
        .unwrap_or_else(|_| "C:/Users/John/Projects/3rdParty/wolfssl".to_string());

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
        .include(format!("{wolfssl}/include"))
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wno-unused-parameter")
        .compile("crypto");

    println!("cargo:rustc-link-search=native=/usr/local/lib");
    println!("cargo:rustc-link-search=native={wolfssl}/lib");
    println!("cargo:rustc-link-search=native={wolfssl}");
    println!("cargo:rustc-link-lib=wolfssl");

    if cfg!(target_os = "windows") {
        println!("cargo:rustc-link-lib=advapi32");
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=ws2_32");
    }
}
