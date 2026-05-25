fn main() {
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/network_wrapper.cpp");
    println!("cargo:rerun-if-changed=src/network_wrapper.h");
    println!("cargo:rerun-if-changed=../../network/");

    if std::env::var("CARGO_FEATURE_CXX_BRIDGE").is_err() {
        return;
    }

    let nlohmann = std::env::var("NLOHMANN_ROOT").unwrap_or_else(|_| {
        "C:/Users/John/Projects/3rdParty/nlohmann_json/include".to_string()
    });

    let cpp_available = std::path::Path::new("../../network").exists();
    let nlohmann_available = std::path::Path::new(&nlohmann).exists()
        || std::path::Path::new("/usr/local/include/nlohmann").exists()
        || std::path::Path::new("/usr/include/nlohmann").exists();

    if !cpp_available || !nlohmann_available {
        println!("cargo:warning=Network C++ library not available, using Rust-only implementation");
        if !nlohmann_available {
            println!("cargo:warning=nlohmann/json.hpp not found. Set NLOHMANN_ROOT or install nlohmann-json.");
        }
        return;
    }

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

        // P2P sources (network calls P2p::serialize, P2p::deserialize, P2p::VarString, P2p::VASize)
        .file("../../p2p/Message.cpp")
        .file("../../p2p/Peer.cpp")
        .file("../../p2p/Serialize.cpp")

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
        .include(&nlohmann)

        // Compiler flags
        .flag_if_supported("-std=c++17")
        .flag_if_supported("-Wno-unused-parameter")
        .compile("network");

    println!("cargo:rustc-link-search=native=/usr/local/lib");
}
