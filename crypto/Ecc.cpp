#include "Ecc.h"

#include "CppUtility.h"
#include "openssl/ec.h"
#include "openssl/evp.h"
#include "openssl/obj_mac.h"

#include <memory>

using namespace Crypto;
using namespace Crypto::Ecc;

static uint8_t const MAX_PRIVATE_KEY[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
    0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40
};

bool Crypto::Ecc::privateKeyIsValid(uint8_t const * k, size_t size)
{
    if (size != PRIVATE_KEY_SIZE)
        return false;

    auto_BIGNUM i(BN_new());
    BN_bin2bn(k, (int)size, i.get());

    if (BN_is_zero(i.get()))
        return false;

    auto_BIGNUM maxPrivateKey(BN_new());
    BN_bin2bn(MAX_PRIVATE_KEY, (int)sizeof(MAX_PRIVATE_KEY), maxPrivateKey.get());

    if (BN_cmp(i.get(), maxPrivateKey.get()) > 0)
        return false;

    return true;
}

bool Crypto::Ecc::derivePublicKey(PrivateKey const & prvKey, PublicKey & pubKey, bool uncompressed /* = false*/)
{
    auto_BIGNUM prvKey_bn(BN_new());
    BN_bin2bn(prvKey.data(), (int)prvKey.size(), prvKey_bn.get());

    auto_EC_GROUP group(EC_GROUP_new_by_curve_name(NID_secp256k1));
    auto_EC_POINT point(EC_POINT_new(group.get()));
    auto_BN_CTX ctx(BN_CTX_new());

    if (!EC_POINT_mul(group.get(), point.get(), prvKey_bn.get(), NULL, NULL, ctx.get()))
        return false;

    pubKey.resize(uncompressed ? UNCOMPRESSED_PUBLIC_KEY_SIZE : COMPRESSED_PUBLIC_KEY_SIZE, 0);
    size_t length = EC_POINT_point2oct(group.get(),
                                       point.get(),
                                       uncompressed ? POINT_CONVERSION_UNCOMPRESSED : POINT_CONVERSION_COMPRESSED,
                                       pubKey.data(),
                                       pubKey.size(),
                                       ctx.get());
    std::rotate(pubKey.begin(), pubKey.begin() + length, pubKey.end());
    return true;
}

bool Crypto::Ecc::sign(uint8_t const *    message,
                       size_t             size,
                       PrivateKey const & prvKey,
                       PublicKey const &  pubKey,
                       Signature &        signature)
{
    signature.clear();

    auto_BIGNUM prvKey_bn(BN_new());
    BN_bin2bn(prvKey.data(), (int)prvKey.size(), prvKey_bn.get());

    auto_EC_KEY ecKey(EC_KEY_new());
    if (!EC_KEY_set_private_key(ecKey.get(), prvKey_bn.get()))
        return false;

    auto_EVP_PKEY pkey(EVP_PKEY_new());
    EVP_PKEY_set1_EC_KEY(pkey.get(), ecKey.get());

    auto_EVP_MD_CTX mdctx(EVP_MD_CTX_create());
    if (!EVP_DigestSignInit(mdctx.get(), NULL, EVP_sha256(), NULL, pkey.get()))
        return false;

    if (!EVP_DigestSignUpdate(mdctx.get(), message, size))
        return false;

    // First call EVP_DigestSignFinal with a NULL sig parameter to obtain the maximum length of the signature.
    size_t signatureSize;
    if (!EVP_DigestSignFinal(mdctx.get(), NULL, &signatureSize))
        return false;

    /* Obtain the signature */
    signature.resize(signatureSize);
    if (!EVP_DigestSignFinal(mdctx.get(), signature.data(), &signatureSize))
        return false;

    // Resize to the actual size
    signature.resize(signatureSize);
    return true;
}

//! Verifies a signed message.
//!
//! @param      message     signed message
//! @param      size        size of the message
//! @param      pubKey      public key
//! @param      signature   signature to verify
//! @return true if the message's signature is vaid and it matches the message
bool Crypto::Ecc::verify(uint8_t const * message, size_t size, PublicKey const & pubKey, Signature const & signature)
{
    auto_EC_GROUP group(EC_GROUP_new_by_curve_name(NID_secp256k1));
    auto_EC_POINT point(EC_POINT_new(group.get()));
    if (!EC_POINT_oct2point(group.get(), point.get(), pubKey.data(), pubKey.size(), NULL))
        return false;

    auto_EC_KEY ecKey(EC_KEY_new());
    if (!EC_KEY_set_public_key(ecKey.get(), point.get()))
        return false;

    auto_EVP_PKEY pkey(EVP_PKEY_new());
    EVP_PKEY_set1_EC_KEY(pkey.get(), ecKey.get());

    auto_EVP_MD_CTX mdctx(EVP_MD_CTX_create());
    if (!EVP_DigestVerifyInit(mdctx.get(), NULL, EVP_sha256(), NULL, pkey.get()))
        return false;

    if (!EVP_DigestVerifyUpdate(mdctx.get(), message, size))
        return false;

    return EVP_DigestVerifyFinal(mdctx.get(), signature.data(), signature.size()) != 0;
}
