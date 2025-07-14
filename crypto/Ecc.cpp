#include "Ecc.h"

#include "Sha256.h"

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/sp_int.h>

#include <cassert>
#include <memory>

using namespace Crypto;
using namespace Crypto::Ecc;

size_t constexpr CURVE_SIZE = PRIVATE_KEY_SIZE;

namespace
{
// Loads a public key into a wolfSSL ecc_key structure.
// Returns true if there were no problems.
static bool loadPublicKey(uint8_t const * k, size_t size, ecc_key * pub)
{
    int rc;

    // Need the curve index to import the public key
    int curveIndex = wc_ecc_get_curve_idx(ECC_SECP256K1);
    assert(curveIndex >= 0);

    // Load the public key into a point
    ecc_point * point = wc_ecc_new_point();
    if (!point)
        return false;

    rc = wc_ecc_import_point_der_ex(k, (word32)size, curveIndex, point, 0);
    if (rc != 0)
    {
        wc_ecc_del_point(point);
        return false;
    }

    uint8_t x[CURVE_SIZE];
    rc = mp_to_unsigned_bin_len(point->x, x, sizeof(x));
    if (rc != MP_OKAY)
    {
        wc_ecc_del_point(point);
        return false;
    }
    uint8_t y[CURVE_SIZE];
    rc = mp_to_unsigned_bin_len(point->y, y, sizeof(y));
    if (rc != MP_OKAY)
    {
        wc_ecc_del_point(point);
        return false;
    }

    wc_ecc_del_point(point);

    rc = wc_ecc_import_unsigned(pub, x, y, NULL, ECC_SECP256K1);
    return rc == 0 && pub->type == ECC_PUBLICKEY;
}

// Loads a private key into a wolfSSL ecc_key structure.
// Returns true if there were no problems.
bool loadPrivateKey(uint8_t const * k, size_t size, ecc_key * prv)
{
    int rc = wc_ecc_import_private_key_ex(k, (word32)size, NULL, 0, prv, ECC_SECP256K1);
    return rc == 0;
}
} // anonymous namespace

bool Crypto::Ecc::publicKeyIsValid(uint8_t const * k, size_t size)
{
    int     rc;
    ecc_key pub;
    rc = wc_ecc_init(&pub);
    assert(rc == 0);

    if (!loadPublicKey(k, size, &pub))
    {
        wc_ecc_free(&pub);
        return false;
    }

    // Make sure it is a valid key
    rc = wc_ecc_check_key(&pub);
    wc_ecc_free(&pub);
    return rc == 0;
}

bool Crypto::Ecc::privateKeyIsValid(uint8_t const * k, size_t size)
{
    int rc;

    // Load the private key
    ecc_key prv;
    rc = wc_ecc_init(&prv);
    assert(rc == 0);

    if (!loadPrivateKey(k, size, &prv))
    {
        wc_ecc_free(&prv);
        return false;
    }

    // Check if it is a valid key
    rc = wc_ecc_check_key(&prv);
    wc_ecc_free(&prv);
    return rc == 0;
}

bool Crypto::Ecc::derivePublicKey(PrivateKey const & prvKey, PublicKey & pubKey, bool uncompressed /* = false*/)
{
    int rc;

    pubKey.clear();

    // Load the private key
    ecc_key prv;
    rc = wc_ecc_init(&prv);
    assert(rc == 0);

    if (!loadPrivateKey(prvKey.data(), prvKey.size(), &prv))
    {
        wc_ecc_free(&prv);
        return false;
    }

    // Derive the public key
    ecc_point * pub = wc_ecc_new_point();
    if (!pub)
    {
        wc_ecc_free(&prv);
        return false;
    }
    rc = wc_ecc_make_pub(&prv, pub);
    if (rc != 0)
    {
        wc_ecc_del_point(pub);
        wc_ecc_free(&prv);
        return false;
    }

    wc_ecc_free(&prv);

    // Export the public key
    word32 outLen     = uncompressed ? UNCOMPRESSED_PUBLIC_KEY_SIZE : COMPRESSED_PUBLIC_KEY_SIZE;
    int    curveIndex = wc_ecc_get_curve_idx(ECC_SECP256K1);
    assert(curveIndex >= 0);

    pubKey.resize(outLen);
    rc = wc_ecc_export_point_der_ex(curveIndex, pub, &pubKey[0], &outLen, (int)!uncompressed);
    if (rc != 0)
    {
        wc_ecc_del_point(pub);
        return false;
    }
    assert(outLen == pubKey.size());
    wc_ecc_del_point(pub);
    return true;
}

bool Crypto::Ecc::sign(uint8_t const * message, size_t size, PrivateKey const & prvKey, Signature & signature)
{
    int rc;

    signature.clear();

    // Hash the message
    Sha256Hash hash = sha256(message, size);

    // Set up the rng
    WC_RNG rng;
    rc = wc_InitRng(&rng);
    assert(rc == 0);

    // Load the private key
    ecc_key prv;
    rc = wc_ecc_init(&prv);
    assert(rc == 0);

    if (!loadPrivateKey(prvKey.data(), (word32)prvKey.size(), &prv))
    {
        wc_ecc_free(&prv);
        wc_FreeRng(&rng);
        return false;
    }

    // Set up r & s
    mp_int r, s;
    rc = mp_init(&r);
    assert(rc == MP_OKAY);
    rc = mp_init(&s);
    assert(rc == MP_OKAY);

    rc = wc_ecc_sign_hash_ex(hash.data(), (word32)hash.size(), &rng, &prv, &r, &s);
    if (rc != 0)
    {
        mp_free(&r);
        mp_free(&s);
        wc_ecc_free(&prv);
        wc_FreeRng(&rng);
        return false;
    }

    wc_ecc_free(&prv);
    wc_FreeRng(&rng);

    /* export r/s */
    size_t rSize = (size_t)mp_unsigned_bin_size(&r);
    assert(rSize == CURVE_SIZE);
    size_t sSize = (size_t)mp_unsigned_bin_size(&s);
    assert(sSize == CURVE_SIZE);
    signature.resize(rSize + sSize);
    mp_to_unsigned_bin(&r, &signature[0]);
    mp_to_unsigned_bin(&s, &signature[rSize]);

    mp_free(&r);
    mp_free(&s);

    return true;
}

bool Crypto::Ecc::verify(uint8_t const * message, size_t size, PublicKey const & pubKey, Signature const & signature)
{
    int rc;

    if (signature.size() != 2 * CURVE_SIZE)
        return false;

    // Hash the message
    Sha256Hash hash = sha256(message, size);

    ecc_key pub;
    rc = wc_ecc_init(&pub);

    assert(rc == 0);
    rc = loadPublicKey(pubKey.data(), pubKey.size(), &pub);
    if (rc != 0)
    {
        wc_ecc_free(&pub);
        return false;
    }

    mp_int r;
    rc = mp_init(&r);
    assert(rc == MP_OKAY);
    rc = mp_read_unsigned_bin(&r, &signature[0], CURVE_SIZE);

    mp_int s;
    rc = mp_init(&s);
    assert(rc == MP_OKAY);
    rc = mp_read_unsigned_bin(&s, &signature[CURVE_SIZE], CURVE_SIZE);

    // Verify the signature
    int verified = 0;
    rc = wc_ecc_verify_hash_ex(&r, &s, hash.data(), (word32)hash.size(), &verified, &pub);
    mp_free(&r);
    mp_free(&s);
    wc_ecc_free(&pub);
    if (rc != 0)
        return false;

    return verified != 0;
}
