fn main() {
    // Check if required C++ libraries are available
    let cpp_available = std::path::Path::new("../../network").exists();
    let nlohmann_available = std::path::Path::new("/usr/local/include/nlohmann").exists() ||
                             std::path::Path::new("/usr/include/nlohmann").exists() ||
                             std::path::Path::new("C:/Users/John/Projects/3rdParty/nlohmann_json/include/nlohmann").exists();
    
    if cpp_available && nlohmann_available {
        println!("cargo:warning=Building Network C++ library with FFI support");
        
        cxx_build::bridge("src/lib.rs")
            // Network core files
            .file("../../network/Address.cpp")
            .file("../../network/Configuration.cpp")
            .file("../../network/Inventory.cpp")
            .file("../../network/Message.cpp")
            .file("../../network/Utility.cpp")
            
            // Message types
            .file("../../network/Messages/AddressMessage.cpp")
            .file("../../network/Messages/AlertMessage.cpp")
            .file("../../network/Messages/BlockMessage.cpp")
            .file("../../network/Messages/CheckOrderMessage.cpp")
            .file("../../network/Messages/FilterAddMessage.cpp")
            .file("../../network/Messages/FilterClearMessage.cpp")
            .file("../../network/Messages/FilterLoadMessage.cpp")
            .file("../../network/Messages/GetAddrMessage.cpp")
            .file("../../network/Messages/GetBlocksMessage.cpp")
            .file("../../network/Messages/GetDataMessage.cpp")
            .file("../../network/Messages/GetHeadersMessage.cpp")
            .file("../../network/Messages/HeadersMessage.cpp")
            .file("../../network/Messages/InventoryMessage.cpp")
            .file("../../network/Messages/MerkleBlockMessage.cpp")
            .file("../../network/Messages/NotFoundMessage.cpp")
            .file("../../network/Messages/PingMessage.cpp")
            .file("../../network/Messages/PongMessage.cpp")
            .file("../../network/Messages/RejectMessage.cpp")
            .file("../../network/Messages/ReplyMessage.cpp")
            .file("../../network/Messages/RequestTransactionsMessage.cpp")
            .file("../../network/Messages/SendHeadersMessage.cpp")
            .file("../../network/Messages/SubmitOrderMessage.cpp")
            .file("../../network/Messages/TransactionMessage.cpp")
            .file("../../network/Messages/VerackMessage.cpp")
            .file("../../network/Messages/VersionMessage.cpp")
            
            // Network wrapper
            .file("src/network_wrapper.cpp")
            
            // Include paths
            .include("../../network")
            .include("../../include")
            .include("../../crypto")
            .include("../../utility")
            .include("../../p2p")
            .include("../../")
            .include("src")
            .include("/usr/local/include")
            .include("/usr/include")
            .include("C:/Users/John/Projects/3rdParty/nlohmann_json/include")  // nlohmann JSON include path
            
            // Compiler flags
            .flag_if_supported("-std=c++17")
            .flag_if_supported("-Wno-unused-parameter")
            .compile("network");

        println!("cargo:rustc-link-search=native=/usr/local/lib");
    } else {
        println!("cargo:warning=Network C++ library not built yet, using Rust-only implementation");
        if !nlohmann_available {
            println!("cargo:warning=nlohmann/json.hpp not found. Install nlohmann-json library for C++ network support");
        }
    }

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/network_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/network_wrapper.h");
    println!("cargo:rerun-if-changed=../../network/");
}
