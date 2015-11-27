#pragma once

#include <cstdint>
#include <vector>

namespace Network
{

    class Address
    {
    public:

        Address(uint32_t time, uint64_t services, std::vector<uint8_t> const & ipv6, uint16_t port);
        Address(uint8_t const *& in, size_t & size);

        void serialize(std::vector<uint8_t> & out) const;

        uint32_t time() const           { return time_; }
        uint64_t services() const       { return services_; }
        uint8_t const * ipv6() const    { return &ipv6_[0]; }
        uint8_t const * ipv4() const    { return &ipv6_[12]; }
        int port() const                { return port_; }

    private:

        uint32_t time_;
        uint64_t services_;
        std::vector<uint8_t> ipv6_;
        uint16_t port_;
    };

} // namespace Network