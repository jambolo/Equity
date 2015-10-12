#pragma once

#include "crypto/Ecdsa.h"
#include "Address.h"
#include <vector>


namespace Equity {

    class Wallet
    {
    public:

        struct Entry
        {
            PrivateKey   privateKey;
            PublicKey    publicKey;
            Address      address;
        };
        typedef std::vector<Entry> EntryList;

        Wallet();
        virtual ~Wallet();


        EntryList entries() const { return entries_; }

    private:

        EntryList entries_;
    };

}