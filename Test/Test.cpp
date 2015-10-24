#include <cstdio>
#include <cstring>
#include <map>
#include <string>

extern int TestCrypto();
extern int TestEquity();
extern int TestNetwork();
extern int TestUtility();
extern int TestValidation();

int main(int argc, char* argv[])
{
    int errors = 0;

    errors += TestUtility();
    errors += TestCrypto();
    errors += TestEquity();
    errors += TestNetwork();
    errors += TestValidation();

    printf("\n%d errors\n", errors);
    return 0;
}
