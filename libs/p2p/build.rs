use std::env;
use std::path::PathBuf;

fn main() {
    // Get the path to the project root
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let manifest_path = PathBuf::from(&manifest_dir);
    let project_root = manifest_path.parent().unwrap().parent().unwrap();
    
    // Check if we have the actual C++ P2P library built (not just headers)
    let p2p_lib_file = project_root.join("build").join("Debug").join("p2p.lib");
    let p2p_lib_file_alt = project_root.join("build").join("p2p").join("Debug").join("p2p.lib");
    let has_built_library = p2p_lib_file.exists() || p2p_lib_file_alt.exists();
    
    // Also check if headers exist
    let p2p_header = project_root.join("p2p").join("Message.h");
    let has_headers = p2p_header.exists();
    
    // Only do full C++ build if we have both headers AND built libraries
    if !has_built_library || !has_headers {
        println!("cargo:warning=P2P C++ library not built yet, using Rust-only implementation");
        return;
    }
    
    // Headers and libraries exist - do full build
    println!("cargo:warning=Building P2P bridge with C++ libraries");
    
    // Link the P2P C++ library
    println!("cargo:rustc-link-search=native={}/build", project_root.display());
    println!("cargo:rustc-link-lib=static=p2p");
    
    // Link dependencies
    println!("cargo:rustc-link-lib=static=utility");
    println!("cargo:rustc-link-lib=static=crypto");
    
    // Platform specific linking
    if cfg!(target_os = "windows") {
        println!("cargo:rustc-link-lib=ws2_32");
        println!("cargo:rustc-link-lib=bcrypt");
    } else if cfg!(target_os = "linux") {
        println!("cargo:rustc-link-lib=pthread");
        println!("cargo:rustc-link-lib=dl");
    } else if cfg!(target_os = "macos") {
        println!("cargo:rustc-link-lib=framework=Security");
    }
    
    // Check if we have a cxx bridge in lib.rs before trying to build it
    let lib_rs_path = PathBuf::from(&manifest_dir).join("src").join("lib.rs");
    if let Ok(content) = std::fs::read_to_string(&lib_rs_path) {
        if content.contains("#[cxx::bridge]") {
            // Build the C++ wrapper
            cxx_build::bridge("src/lib.rs")
                .file("src/p2p_wrapper.cpp")
                .include(project_root.join("p2p"))
                .include(project_root.join("include"))
                .include(project_root.join("utility"))
                .include(project_root.join("crypto"))
                .flag_if_supported("-std=c++17")
                .flag_if_supported("-O2")
                .compile("p2p_bridge");
        }
    }
    
    // Tell cargo to rerun if any of these change
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/p2p_wrapper.h");
    println!("cargo:rerun-if-changed=src/p2p_wrapper.cpp");
    println!("cargo:rerun-if-changed=../../p2p/");
}
