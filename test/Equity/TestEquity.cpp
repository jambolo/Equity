#include <cstdio>

extern int TestAddress();
extern int TestBase58();
extern int TestBase58Check();
extern int TestPrivateKey();
extern int TestPublicKey();

int TestEquity()
{
    printf("Equity\n");

    int errors = 0;
    errors += TestAddress();
    errors += TestBase58();
    errors += TestBase58Check();
    errors += TestPrivateKey();
    errors += TestPublicKey();
    return errors;
}