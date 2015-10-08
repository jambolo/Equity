#pragma once

#include "crypto/Ecdsa.h"
#include "BitcoinAddress.h"
#include <vector>


namespace Equity {

    class Wallet
    {
    public:

        struct Entry
        {
            Crypto::Ecdsa::PrivateKey   privateKey;
            Crypto::Ecdsa::PublicKey    publicKey;
            BitcoinAddress              address;
        };
        typedef std::vector<Entry> EntryList;

        Wallet();
        virtual ~Wallet();


        EntryList entries() const { return entries_; }

    private:

        EntryList entries_;
    };

}