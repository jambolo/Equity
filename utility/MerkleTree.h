#pragma once

#include "crypto/Sha256.h"
#include <vector>

namespace Utility
{
class MerkleTree
{
public:

    MerkleTree(std::vector<Crypto::Sha256Hash> const & hashes);

    Crypto::Sha256Hash root() const;
    Crypto::Sha256Hash hashAt(size_t i) const;
    std::vector<Crypto::Sha256Hash> proof(size_t i) const;

    //! Verifies that the given hash is the hash for the ith item given a proof and a root
    static bool verify(Crypto::Sha256Hash const &              hash,
                       size_t                                  i,
                       std::vector<Crypto::Sha256Hash> const & proof,
                       Crypto::Sha256Hash const &              root);

private:

    static size_t const ROOT = 1;   // Yes, 1 is correct

    std::vector<Crypto::Sha256Hash> tree_;
    size_t nLeaves_;                        // Number of leaf nodes
    size_t offset_;                         // Offset to the first leaf node
};
} // namespace Utility
