#include <cstdio>
#include <cstring>
#include <map>

extern int TestBase58();
extern int TestBase58Check();
//extern int TestPrivateKey();
extern int TestPublicKey();
extern int TestAddress();

typedef std::map<std::string, int(*)()> TestMap;

static void addTests(TestMap & tests);

int main(int argc, char* argv[])
{
    TestMap tests;
    addTests(tests);

    int errors = 0;
    for (int i = 1; i < argc; ++i)
    {
        char * name = argv[i];
        TestMap::iterator p = tests.find(name);
        if (p != tests.end())
        {
            errors += (p->second)();
        }
        else
        {
            printf("'%s' is not a valid test name.\n", name);
        }
    }

    printf("\n%d errors\n", errors);
    return 0;
}

static void addTests(TestMap & tests)
{
    tests["Base58"] = TestBase58;
    tests["Base58Check"] = TestBase58Check;
//    tests["PrivateKey"] = TestPrivateKey;
    tests["PublicKey"] = TestPublicKey;
    tests["Address"] = TestAddress;
}