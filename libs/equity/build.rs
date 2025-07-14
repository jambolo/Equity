use std::env;
use std::path::PathBuf;

fn main() {
    // Get the path to the project root
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let manifest_path = PathBuf::from(&manifest_dir);
    let project_root = manifest_path.parent().unwrap().parent().unwrap();
    
    // Check if we have the actual C++ Equity library built (not just headers)
    let equity_lib_file = project_root.join("build").join("Debug").join("equity.lib");
    let equity_lib_file_alt = project_root.join("build").join("equity").join("Debug").join("equity.lib");
    let has_built_library = equity_lib_file.exists() || equity_lib_file_alt.exists();
    
    // Also check if headers exist
    let equity_header = project_root.join("equity").join("Address.cpp");
    let has_headers = equity_header.exists();
    
    // Only do full C++ build if we have both headers AND built libraries
    if !has_built_library || !has_headers {
        println!("cargo:warning=Equity C++ library not built yet, using Rust-only implementation");
        return;
    }
    
    // Headers and libraries exist - do full build
    println!("cargo:warning=Building Equity bridge with C++ libraries");
    
    // Check if we have a cxx bridge in lib.rs before trying to build it
    let lib_rs_path = PathBuf::from(&manifest_dir).join("src").join("lib.rs");
    if let Ok(content) = std::fs::read_to_string(&lib_rs_path) {
        if content.contains("#[cxx::bridge]") {
            cxx_build::bridge("src/lib.rs")
                .file("src/equity_wrapper.cpp")
                .include(project_root.join("equity"))
                .include(project_root.join("include"))
                .include(project_root.join("crypto"))
                .include(project_root.join("utility"))
                .include(project_root.join("network"))
                .include(project_root.join("p2p"))
                .include(project_root)
                .flag_if_supported("-std=c++17")
                .flag_if_supported("-Wno-unused-parameter")
                .compile("equity");
        }
    }

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/equity_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/equity_wrapper.h");
    println!("cargo:rerun-if-changed=../../equity/");
}
