#pragma once

#include <cstdint>
#include <cstddef>
#include "rust/cxx.h"

// C wrapper function declarations for cxx bridge compatibility

extern "C" {

// Configuration functions
uint32_t networkConfigurationGetNetwork();
int networkConfigurationGetPort();

// Address functions
bool networkAddressCreate(uint32_t time, uint64_t services, const uint8_t* ipv6, uint16_t port, rust::Vec<uint8_t>& serialized);
bool networkAddressDeserialize(const rust::Vec<uint8_t>& data, uint32_t& time, uint64_t& services, rust::Vec<uint8_t>& ipv6, uint16_t& port);
bool networkAddressSerialize(uint32_t time, uint64_t services, const rust::Vec<uint8_t>& ipv6, uint16_t port, rust::Vec<uint8_t>& output);
bool networkAddressToJson(uint32_t time, uint64_t services, const rust::Vec<uint8_t>& ipv6, uint16_t port, rust::String& json);

// Inventory functions
bool networkInventoryCreate(uint32_t type, const rust::Vec<uint8_t>& hash, rust::Vec<uint8_t>& serialized);
bool networkInventoryDeserialize(const rust::Vec<uint8_t>& data, uint32_t& type, rust::Vec<uint8_t>& hash);
bool networkInventorySerialize(uint32_t type, const rust::Vec<uint8_t>& hash, rust::Vec<uint8_t>& output);
bool networkInventoryToJson(uint32_t type, const rust::Vec<uint8_t>& hash, rust::String& json);

// Message base functions
bool networkMessageGetType(const rust::Vec<uint8_t>& serialized, rust::String& type);

// Version Message functions
bool networkVersionMessageCreate(
    uint32_t version, uint64_t services, uint64_t timestamp,
    // to address
    uint32_t to_time, uint64_t to_services, const rust::Vec<uint8_t>& to_ipv6, uint16_t to_port,
    // from address  
    uint32_t from_time, uint64_t from_services, const rust::Vec<uint8_t>& from_ipv6, uint16_t from_port,
    uint64_t nonce, rust::Str user_agent, uint32_t height, bool relay,
    rust::Vec<uint8_t>& serialized
);

bool networkVersionMessageDeserialize(
    const rust::Vec<uint8_t>& data,
    uint32_t& version, uint64_t& services, uint64_t& timestamp,
    // to address
    uint32_t& to_time, uint64_t& to_services, rust::Vec<uint8_t>& to_ipv6, uint16_t& to_port,
    // from address
    uint32_t& from_time, uint64_t& from_services, rust::Vec<uint8_t>& from_ipv6, uint16_t& from_port,
    uint64_t& nonce, rust::String& user_agent, uint32_t& height, bool& relay
);

// Address Message functions - using serialized format for multiple addresses
bool networkAddressMessageCreate(const rust::Vec<uint8_t>& serialized_addresses, rust::Vec<uint8_t>& serialized);
bool networkAddressMessageDeserialize(const rust::Vec<uint8_t>& data, rust::Vec<uint8_t>& serialized_addresses);

// Inventory Message functions - using serialized format for multiple inventories
bool networkInventoryMessageCreate(const rust::Vec<uint8_t>& serialized_inventories, rust::Vec<uint8_t>& serialized);
bool networkInventoryMessageDeserialize(const rust::Vec<uint8_t>& data, rust::Vec<uint8_t>& serialized_inventories);

// GetData Message functions - using serialized format for multiple inventories
bool networkGetDataMessageCreate(const rust::Vec<uint8_t>& serialized_inventories, rust::Vec<uint8_t>& serialized);
bool networkGetDataMessageDeserialize(const rust::Vec<uint8_t>& data, rust::Vec<uint8_t>& serialized_inventories);

// GetBlocks Message functions - using serialized format for multiple hashes
bool networkGetBlocksMessageCreate(
    uint32_t version, const rust::Vec<uint8_t>& serialized_hashes, const rust::Vec<uint8_t>& stop_hash,
    rust::Vec<uint8_t>& serialized
);

// Ping Message functions
bool networkPingMessageCreate(uint64_t nonce, rust::Vec<uint8_t>& serialized);
bool networkPingMessageDeserialize(const rust::Vec<uint8_t>& data, uint64_t& nonce);

// Pong Message functions  
bool networkPongMessageCreate(uint64_t nonce, rust::Vec<uint8_t>& serialized);
bool networkPongMessageDeserialize(const rust::Vec<uint8_t>& data, uint64_t& nonce);

// Simple message types (no payload)
bool networkVerackMessageCreate(rust::Vec<uint8_t>& serialized);
bool networkGetAddrMessageCreate(rust::Vec<uint8_t>& serialized);
bool networkFilterClearMessageCreate(rust::Vec<uint8_t>& serialized);
bool networkSendHeadersMessageCreate(rust::Vec<uint8_t>& serialized);

// Generic message functions
bool networkMessageSerialize(const rust::String& command, const rust::Vec<uint8_t>& payload, uint32_t magic, rust::Vec<uint8_t>& output);
bool networkMessageDeserialize(const rust::Vec<uint8_t>& data, rust::String& command, rust::Vec<uint8_t>& payload, uint32_t& magic, uint32_t& checksum);
bool networkMessageToJson(const rust::String& command, const rust::Vec<uint8_t>& payload, uint32_t magic, rust::String& json);
bool networkMessageValidate(const rust::String& command, const rust::Vec<uint8_t>& payload, uint32_t magic, uint32_t checksum);

// Message creation helpers
bool networkCreatePingMessage(uint64_t nonce, rust::Vec<uint8_t>& payload);
bool networkCreatePongMessage(uint64_t nonce, rust::Vec<uint8_t>& payload);

} // extern "C"
