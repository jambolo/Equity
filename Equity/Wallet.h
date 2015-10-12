#pragma once

#include <vector>
#include "PrivateKey.h"
#include "PublicKey.h"
#include "Address.h"


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