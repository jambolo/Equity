#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

// Forward declarations
namespace P2P {
    class Message;
    class Peer;
}

// Opaque C++ types for FFI bridge (cxx will generate these)
struct MessageCpp;
struct PeerCpp;

// C++ struct types that can be used directly
struct MessageHeaderCpp {
    uint32_t magic;
    std::string type_name;
    uint32_t length;
    uint32_t checksum;
};

// Result structs for complex deserialization
struct VarIntResult {
    uint64_t value;
    size_t bytes_read;
};

struct VarStringResult {
    std::string value;
    size_t bytes_read;
};

// Message functions
extern "C" {
    // Message creation and management
    std::unique_ptr<MessageCpp> messageCreate(const char* type, const uint8_t* payload, size_t payload_len);
    std::string messageGetType(const MessageCpp& message);
    std::vector<uint8_t> messageGetPayload(const MessageCpp& message);
    size_t messageGetSize(const MessageCpp& message);
    std::string messageToJson(const MessageCpp& message);

    // Message header functions
    MessageHeaderCpp messageHeaderCreate(uint32_t magic, const char* type, uint32_t length, uint32_t checksum);
    MessageHeaderCpp messageHeaderFromData(const uint8_t* data, size_t data_len);
    std::vector<uint8_t> messageHeaderSerialize(const MessageHeaderCpp& header);
    std::string messageHeaderToJson(const MessageHeaderCpp& header);

    // Peer creation and management
    std::unique_ptr<PeerCpp> peerCreate(const char* address);
    std::unique_ptr<PeerCpp> peerFromConnection(const char* address, uint32_t connection_id);
    std::string peerGetAddress(const PeerCpp& peer);
    int peerGetConnectionStatus(const PeerCpp& peer);
    uint64_t peerGetServices(const PeerCpp& peer);
    void peerSetServices(PeerCpp& peer, uint64_t services);
    uint32_t peerGetProtocolVersion(const PeerCpp& peer);
    void peerSetProtocolVersion(PeerCpp& peer, uint32_t version);
    std::string peerGetUserAgent(const PeerCpp& peer);
    void peerSetUserAgent(PeerCpp& peer, const char* user_agent);
    uint32_t peerGetStartHeight(const PeerCpp& peer);
    void peerSetStartHeight(PeerCpp& peer, uint32_t height);
    uint64_t peerGetLastSeen(const PeerCpp& peer);
    void peerSetLastSeen(PeerCpp& peer, uint64_t timestamp);
    uint64_t peerGetConnectionTime(const PeerCpp& peer);
    bool peerConnect(PeerCpp& peer);
    void peerDisconnect(PeerCpp& peer);
    bool peerSendMessage(PeerCpp& peer, const MessageCpp& message);
    bool peerHasPendingMessages(const PeerCpp& peer);
    std::unique_ptr<MessageCpp> peerReceiveMessage(PeerCpp& peer);
    uint64_t peerGetBytesSent(const PeerCpp& peer);
    uint64_t peerGetBytesReceived(const PeerCpp& peer);
    uint64_t peerGetMessagesSent(const PeerCpp& peer);
    uint64_t peerGetMessagesReceived(const PeerCpp& peer);
    std::string peerToJson(const PeerCpp& peer);

    // Serialization functions
    std::vector<uint8_t> serializeU16(uint16_t value);
    std::vector<uint8_t> serializeU32(uint32_t value);
    std::vector<uint8_t> serializeU64(uint64_t value);
    std::vector<uint8_t> serializeI16(int16_t value);
    std::vector<uint8_t> serializeI32(int32_t value);
    std::vector<uint8_t> serializeI64(int64_t value);
    std::vector<uint8_t> serializeVarInt(uint64_t value);
    std::vector<uint8_t> serializeVarString(const char* value);

    uint16_t deserializeU16(const uint8_t* data);
    uint32_t deserializeU32(const uint8_t* data);
    uint64_t deserializeU64(const uint8_t* data);
    int16_t deserializeI16(const uint8_t* data);
    int32_t deserializeI32(const uint8_t* data);
    int64_t deserializeI64(const uint8_t* data);
    VarIntResult deserializeVarInt(const uint8_t* data, size_t data_len);
    VarStringResult deserializeVarString(const uint8_t* data, size_t data_len);
}
