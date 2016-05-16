#pragma once

#include "Address.h"
#include "PrivateKey.h"
#include "PublicKey.h"
#include <vector>

namespace Equity
{

//! A wallet holds private keys and other metadata related to those keys.
class Wallet
{
public:

    //! An wallet entry containing information about a private key.
    struct Entry
    {
        PrivateKey privateKey;  //!< The private key
        PublicKey publicKey;    //!< The public key derived from the private key
        Address address;        //!< The address derived from the public key
    };

    //! A list of entries in the wallet
    typedef std::vector<Entry> EntryList;

    // Constructor
    Wallet();

    // Destructor
    virtual ~Wallet();

    //! Returns the list of keys contained in the wallet
    EntryList entries() const { return entries_; }

private:

    EntryList entries_;
};

} // namespace Equity
