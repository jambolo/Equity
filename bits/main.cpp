//
//  main.cpp
//  Equity
//
//  Created by John Bolton on 10/16/15.
//
//

#include "equity/Target.h"
#include "utility/Utility.h"

#include <cstdio>

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

    unsigned bits;
    if (1 != sscanf(*argv, "%u", &bits))
        return 2;

    Equity::Target t(bits);
    printf("Bits = %u (0x%08x)\n", bits, bits);
    printf("Hash = %s\n", Utility::toHex(t).c_str());
    printf("Difficulty = %.0lf\n", t.difficulty());
    return 0;
}

static void syntax(int argc, char ** argv)
{
    fprintf(stderr, "syntax: %s <value in decimal>\n", argv[0]);
}
