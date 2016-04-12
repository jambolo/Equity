#pragma once

#include "network/Address.h"
#include "network/Message.h"

namespace Network
{

//! A version message
//!
//! When a node creates an outgoing connection, it will immediately advertise its version. The remote node will respond with its
//! version. No further communication is possible until both peers have exchanged their version.
//!
//! @sa https://github.com/bitcoin/bips/blob/master/bip-0037.mediawiki
//! @todo more work to do with this class

class VersionMessage : public Message
{
public:
    // Constructor
    VersionMessage::VersionMessage(uint32_t            version,
                                   uint64_t            services,
                                   uint64_t            timestamp,
                                   Address const &     to,
                                   Address const &     from,
                                   uint64_t            nonce,
                                   std::string const & userAgent,
                                   uint32_t            height,
                                   bool                relay);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VersionMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    uint32_t version_;          //!< Identifies protocol version being used by the node
    uint64_t services_;         //!< bitfield of features to be enabled for this connection
    uint64_t timestamp_;        //!< standard UNIX timestamp in seconds
    Address to_;                //!< The network address of the node receiving this message

    // Fields below require version >= 106
    Address from_;              //!< The network address of the node emitting this message
    uint64_t nonce_;            //!< Node random nonce, randomly generated every time a version packet is sent. This nonce is used
                                //!< to detect connections to self.
    std::string userAgent_;     //!< User Agent(0x00 if string is 0 bytes long)
    uint32_t height_;           //!< The last block received by the emitting node

    // Fields below require version >= 70001
    bool relay_;                //!< Whether the remote peer should announce relayed transactions or not, see BIP 0037

    //! Message type
    static char const TYPE[];
};

} // namespace Network
