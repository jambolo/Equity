#pragma once

#include "p2p/Serialize.h"
#include <array>
#include <cstdint>

namespace Network
{
//! @ingroup NetworkGroup
//!@{

//! A description of a network node.
//!
//! @sa     VersionMessage, AddressMessage

class Address : public P2p::Serializable
{
public:
    // Constructor
    Address();

    // Constructor
    //!
    //! @param  time        Timestamp
    //! @param  services    Services provided by the node
    //! @param  ipv6        IP address
    //! @param  port        Port
    Address(uint32_t time, uint64_t services, std::array<uint8_t, 16> const & ipv6, uint16_t port);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    Address(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    //! Returns the timestamp
    uint32_t time() const { return time_; }

    //! Returns the services
    uint64_t services() const { return services_; }

    //! Returns the IP address in IPV6 form
    uint8_t const * ipv6() const { return ipv6_.data(); }

    //! Returns the IP address in IPV4 form
    uint8_t const * ipv4() const { return ipv6_.data() + 12; }

    //! Returns the port
    int port() const { return port_; }

private:

    uint32_t time_;
    uint64_t services_;
    std::array<uint8_t, 16> ipv6_;
    uint16_t port_;
};

//!@}
} // namespace Network
