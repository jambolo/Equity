//
//  main.cpp
//  Equity
//
//  Created by John Bolton on 10/16/15.
//
//

#include "equity/Transaction.h"
#include "utility/Serialize.h"
#include "utility/Utility.h"

#include <cstdio>
#include <memory>
#include <vector>

static void syntax(int argc, char ** argv);

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        syntax(argc, argv);
        return 1;
    }

    --argc;
    ++argv;

//    std::string hash =
// "0100000002f327e86da3e66bd20e1129b1fb36d07056f0b9a117199e759396526b8f3a20780000000000fffffffff0ede03d75050f20801d50358829ae02c058e8677d2cc74df51f738285013c260000000000ffffffff02f028d6dc010000001976a914ffb035781c3c69e076d48b60c3d38592e7ce06a788ac00ca9a3b000000001976a914fa5139067622fd7e1e722a05c17c2bb7d5fd6df088ac00000000";//
// *argv;
    std::string hash = "0100000001484d40d45b9ea0d652fca8258ab7caa42541eb52975857f96fb50cd732c8b481000000008a47304402202cb265bf10707bf49346c3515dd3d16fc454618c58ec0a0ff448a676c54ff71302206c6624d762a1fcef4618284ead8f08678ac05b13c84235f1654e6ad168233e8201410414e301b2328f17442c0b8310d787bf3d8a404cfbd0704f135b6ad4b2d3ee751310f981926e53a6e8c39bd7d3fefd576c543cce493cbac06388f2651d1aacbfcdffffffff0162640100000000001976a914c8e90996c7c6080ee06284600c684ed904d14c5c88ac00000000";
    std::vector<uint8_t> data = Utility::fromHex(hash);
    uint8_t const * in = &data[0];
    size_t size = data.size();
    try
    {
        Equity::Transaction transaction(in, size);
        std::string json = transaction.toJson();
        printf("%s\n", json.c_str());
    }
    catch (Utility::DeserializationError const & e)
    {
        size_t offset = data.size() - size;
        std::string start = Utility::toHex(&data[offset], size);
        fprintf(stderr, "Invalid transaction. Failed around offset %u (%s)\n", (unsigned)offset, Utility::shorten(start, 7).c_str());
        return 2;
    }
    return 0;
}

static void syntax(int argc, char ** argv)
{
    fprintf(stderr, "syntax: %s <hash>\n", argv[0]);
}
