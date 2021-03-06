//
//  main.cpp
//  Equity
//
//  Created by John Bolton on 10/16/15.
//
//

#include "crypto/Ripemd.h"
#include "crypto/Sha256.h"
#include "equity/Base58Check.h"

#include <cstdio>
#include <vector>

int main(int argc, char ** argv)
{
    {
        Crypto::Ripemd160Hash hashLow;
        Crypto::Ripemd160Hash hashHigh;
        hashLow.fill(0);
        hashHigh.fill(0xff);
        for (int i = 0; i < 256; ++i)
        {
            std::string low = Equity::Base58Check::encode(hashLow, i);
            std::string high = Equity::Base58Check::encode(hashHigh, i);

            printf("0x%02x: low: %-35s high: %-35s\n", i, low.c_str(), high.c_str());
        }
    }

    {
        Crypto::Sha256Hash hashLow;
        Crypto::Sha256Hash hashHigh;
        hashLow.fill(0);
        hashHigh.fill(0xff);
        for (int i = 0; i < 256; ++i)
        {
            std::string low = Equity::Base58Check::encode(hashLow, i);
            std::string high = Equity::Base58Check::encode(hashHigh, i);

            printf("0x%02x: low: %-51s high: %-51s\n", i, low.c_str(), high.c_str());
        }
    }

}
