//
//  main.cpp
//  Equity
//
//  Created by John Bolton on 10/16/15.
//
//

#include "equity/Script.h"
#include "equity/Transaction.h"
#include "p2p/Serialize.h"
#include "utility/Utility.h"

#include <cstdio>
#include <memory>
#include <vector>

using namespace Equity;

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

// 0100000001484d40d45b9ea0d652fca8258ab7caa42541eb52975857f96fb50cd732c8b481000000008a47304402202cb265bf10707bf49346c3515dd3d16fc454618c58ec0a0ff448a676c54ff71302206c6624d762a1fcef4618284ead8f08678ac05b13c84235f1654e6ad168233e8201410414e301b2328f17442c0b8310d787bf3d8a404cfbd0704f135b6ad4b2d3ee751310f981926e53a6e8c39bd7d3fefd576c543cce493cbac06388f2651d1aacbfcdffffffff0162640100000000001976a914c8e90996c7c6080ee06284600c684ed904d14c5c88ac00000000
// 0100000002f327e86da3e66bd20e1129b1fb36d07056f0b9a117199e759396526b8f3a20780000000000fffffffff0ede03d75050f20801d50358829ae02c058e8677d2cc74df51f738285013c260000000000ffffffff02f028d6dc010000001976a914ffb035781c3c69e076d48b60c3d38592e7ce06a788ac00ca9a3b000000001976a914fa5139067622fd7e1e722a05c17c2bb7d5fd6df088ac00000000
// 01000000010c432f4fb3e871a8bda638350b3d5c698cf431db8d6031b53e3fb5159e59d4a9000000006b48304502201123d735229382f75496e84ae5831871796ef78726805adc2c6edd36d23e7210022100faceab822a4943309c4b6b61240ae3a9e18ed90a75117c5dc4bfd8f7e17a21d301210367ce0a1c3b3e84cece6dad1a181d989d8e490b84f5431a1f778a88b284c935e6ffffffff0100f2052a010000001976a9143744841e13b90b4aca16fe793a7f88da3a23cc7188ac00000000
    std::vector<uint8_t> data = Utility::fromHex(*argv);
    uint8_t const * in = &data[0];
    size_t size = data.size();
    try
    {
        Transaction transaction(in, size);
        printf("valid: %s\n", transaction.valid() ? "true" : "false");
        printf("version: %u\n", transaction.version());

        Transaction::InputList inputs = transaction.inputs();
        printf("inputs:\n");
        for (size_t i = 0; i < inputs.size(); ++i)
        {
            Transaction::Input const & input = inputs[i];
            Script script(input.script);
            printf("    %2zu:     txid : %s\n", i, Utility::toHex(input.txid.hash_).c_str());
            printf("           index : %u\n", input.outputIndex);
            printf("          script : %s\n", script.toSource().c_str());
            printf("        sequence : %u\n", input.sequence);
        }

        Transaction::OutputList outputs = transaction.outputs();
        printf("outputs:\n");
        for (size_t i = 0; i < outputs.size(); ++i)
        {
            Transaction::Output const & output = outputs[i];
            Script script(output.script);
            printf("    %2zu:   value : %llu (%lf BTC)\n", i, output.value, (double)output.value / 100000000.0);
            printf("         script : %s\n", script.toSource().c_str());
        }

        printf("lockTime: %u\n", transaction.lockTime());
    }
    catch (P2p::DeserializationError)
    {
        fprintf(stderr, "Invalid transaction.\n");
        return 2;
    }
    return 0;
}

static void syntax(int argc, char ** argv)
{
    fprintf(stderr, "syntax: %s <hash>\n", argv[0]);
}
