#pragma once

#include "crypto/Sha256.h"
#include <vector>

namespace Utility
{
//! @addtogroup UtilityGroup
//!@{

//! A Merkle Tree implementation with all the idiosyncracies specific to Bitcoin
class MerkleTree
{
public:

    // Constructor
    //! @param  hashes  ordered list of hashes
    MerkleTree(Crypto::Sha256HashList const & hashes);

    //! Returns the root
    Crypto::Sha256Hash root() const;

    //! Returns the hash at the given index.
    //!
    //! @param  i       index
    Crypto::Sha256Hash hashAt(size_t i) const;

    //! Returns a proof for the hash at the given index.
    //!
    //! @param  i       index
    Crypto::Sha256HashList proof(size_t i) const;

    //! Verifies that the given hash is the hash at the given index, given a proof and a root.
    //!
    //! @param  hash    hash to verify
    //! @param  i       location of the hash
    //! @param  proof   list of hashes in depth-first order used to verify the hash
    //! @param  root    root value that should be the result of the proof
    static bool verify(Crypto::Sha256Hash const &     hash,
                       size_t                         i,
                       Crypto::Sha256HashList const & proof,
                       Crypto::Sha256Hash const &     root);

private:

    static size_t const ROOT = 1;   // Yes, 1 is correct

    Crypto::Sha256HashList tree_;
    size_t nLeaves_;                        // Number of leaf nodes
    size_t offset_;                         // Offset to the first leaf node
};

//!@}
} // namespace Utility
