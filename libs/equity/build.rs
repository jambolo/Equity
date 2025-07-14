fn main() {
    cxx_build::bridge("src/lib.rs")
        .file("src/equity_wrapper.cpp")
        .include("../../equity")
        .include("../../include")
        .include("../../crypto")
        .include("../../utility")
        .include("../../network")
        .include("../../p2p")
        .include("../../")
        .include("src")
        .include("/usr/local/include")
        .include("/usr/include")
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wno-unused-parameter")
        .compile("equity");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/equity_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/equity_wrapper.h");
    println!("cargo:rerun-if-changed=../../equity/");
    println!("cargo:rustc-link-search=native=/usr/local/lib");
    println!("cargo:rustc-link-lib=wolfssl");
    println!("cargo:rustc-link-lib=stdc++");
}
