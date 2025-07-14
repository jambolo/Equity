fn main() {
    // Check if required C++ libraries are available
    let cpp_available = std::path::Path::new("../../utility").exists() && 
                        std::path::Path::new("../../crypto").exists();
    
    if cpp_available {
        println!("cargo:warning=Building Utility C++ library with FFI support");
        
        cxx_build::bridge("src/lib.rs")
            // Utility core files
            .file("../../utility/Endian.cpp")
            .file("../../utility/Utility.cpp")
            
            // Utility wrapper
            .file("src/utility_wrapper.cpp")
            
            // Include paths
            .include("../../utility")
            .include("src")
            .include("/usr/local/include")
            .include("/usr/include")
            .include("C:/Users/John/Projects/3rdParty/wolfssl/include")  // Add WolfSSL include path
            
            // Compiler flags
            .flag_if_supported("-std=c++17")
            .flag_if_supported("-Wno-unused-parameter")
            .compile("utility");

        println!("cargo:rustc-link-search=native=/usr/local/lib");
        println!("cargo:rustc-link-search=native=C:/Users/John/Projects/3rdParty/wolfssl/lib");
        println!("cargo:rustc-link-lib=wolfssl");
    } else {
        println!("cargo:warning=Utility C++ library not built yet, using Rust-only implementation");
    }

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/utility_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/utility_wrapper.h");
    println!("cargo:rerun-if-changed=../../utility/");
}
