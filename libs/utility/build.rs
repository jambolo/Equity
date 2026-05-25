fn main() {
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/utility_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/utility_wrapper.h");
    println!("cargo:rerun-if-changed=../../utility/");

    if std::env::var("CARGO_FEATURE_CXX_BRIDGE").is_err() {
        return;
    }

    let cpp_available = std::path::Path::new("../../utility").exists()
        && std::path::Path::new("../../crypto").exists();

    if !cpp_available {
        println!("cargo:warning=Utility C++ sources not found, using Rust-only implementation");
        return;
    }

    let wolfssl = std::env::var("WOLFSSL_ROOT")
        .unwrap_or_else(|_| "C:/Users/John/Projects/3rdParty/wolfssl".to_string());

    println!("cargo:warning=Building Utility C++ library with FFI support");

    cxx_build::bridge("src/lib.rs")
        .file("../../utility/Endian.cpp")
        .file("../../utility/Utility.cpp")
        .file("src/utility_wrapper.cpp")
        .include("../../utility")
        .include("src")
        .include("/usr/local/include")
        .include("/usr/include")
        .include(format!("{wolfssl}/include"))
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wno-unused-parameter")
        .compile("utility");

    println!("cargo:rustc-link-search=native=/usr/local/lib");
    println!("cargo:rustc-link-search=native={wolfssl}/lib");
    println!("cargo:rustc-link-lib=wolfssl");
}
