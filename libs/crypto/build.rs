fn main() {
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/crypto_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/crypto_wrapper.h");
    println!("cargo:rerun-if-changed=../../crypto/");

    if std::env::var("CARGO_FEATURE_CXX_BRIDGE").is_err() {
        return;
    }

    cxx_build::bridge("src/lib.rs")
        .file("src/crypto_wrapper.cpp")
        .include("../../crypto")
        .include("../../include")
        .include("../../utility")
        .include("../../")
        .include("src")
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wno-unused-parameter")
        .compile("crypto");
}
