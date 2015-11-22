#include "MerkleTree.h"

#include "crypto/Sha256.h"

namespace
{

// Returns the smallest power of two >= x
size_t smallestPowerOfTwo(size_t x)
{
    while ((x & (x - 1)) != 0)
    {
        x = (x | (x - 1)) + 1;
    }
    return x;
}

// Returns true if x is even
bool even(size_t x)
{
    return (x & 1) == 0;
}

size_t parentOf(size_t i) { return i / 2; }
size_t siblingOf(size_t i) { return i ^ 1; }
size_t leftChildOf(size_t i) { return i * 2; }
size_t rightChildOf(size_t i) { return i * 2 + 1; }

std::vector<uint8_t> concatenate(Crypto::Sha256Hash const & left, Crypto::Sha256Hash const & right)
{
    std::vector<uint8_t> concatenated;
    concatenated.reserve(2 * Crypto::SHA256_HASH_SIZE);
    concatenated = left;
    concatenated.insert(concatenated.end(), right.begin(), right.end());
    return concatenated;
}

} // anonymous namespace

namespace Utility
{

MerkleTree::MerkleTree(Crypto::Sha256HashList const & hashes)
{
    // Figure out the size of the tree
    nLeaves_ = hashes.size();

    if (nLeaves_ > 1)
    {
        // An even number of leaves is necessary. In the odd case, the last is duplicated.
        size_t paddedNLeaves = (nLeaves_ + 1) & ~(size_t)1;

        offset_ = smallestPowerOfTwo(paddedNLeaves);
        tree_.reserve(offset_ + paddedNLeaves);

        // Make space for interior nodes
        tree_.resize(offset_);

        // Load the leaves (and duplicate the last one if there is an odd number)
        tree_.insert(tree_.end(), hashes.begin(), hashes.end());
        if (!even(nLeaves_))
        {
            tree_.push_back(tree_.back());
        }

        // Calculate the interior of the tree
        size_t n = paddedNLeaves / 2;
        size_t first = parentOf(offset_);
        while (n > 0)
        {
            for (size_t i = first; i < first + n; ++i)
            {
                tree_[i] = Crypto::doubleSha256(concatenate(tree_[leftChildOf(i)], tree_[rightChildOf(i)]));
            }
            // Duplicate the last one if there is an odd number (except for the root!)
            if (!even(n) && first > ROOT)
            {
                tree_[first + n] = tree_[first + n - 1];
                ++n;
            }

            n /= 2;
            first = parentOf(first);
        }
    }
    // Special case: If there is only one leaf, then it is the root (very odd)
    else
    {
        offset_ = 1;
        tree_.resize(offset_);
        tree_.push_back(hashes[0]);
    }
}

Crypto::Sha256Hash MerkleTree::root() const
{
    return tree_[ROOT];
}

Crypto::Sha256Hash MerkleTree::hashAt(size_t i) const
{
    if (i >= nLeaves_)
    {
        return Crypto::Sha256Hash();
    }

    return tree_[offset_ + i];
}

Crypto::Sha256HashList MerkleTree::proof(size_t i) const
{
    if (i >= nLeaves_)
    {
        return Crypto::Sha256HashList();
    }

    i += offset_;

    Crypto::Sha256HashList p;
    while (i > ROOT)
    {
        p.push_back(tree_[siblingOf(i)]);
        i = parentOf(i);
    }
    return p;
}

bool MerkleTree::verify(Crypto::Sha256Hash const &              hash,
                        size_t                                  i,
                        Crypto::Sha256HashList const & proof,
                        Crypto::Sha256Hash const &              root)
{
    Crypto::Sha256Hash result = hash;
    for (auto const & p: proof)
    {
        if (even(i))
        {
            result = Crypto::doubleSha256(concatenate(result, p));
        }
        else
        {
            result = Crypto::doubleSha256(concatenate(p, result));
        }
        i /= 2;
    }

    return result == root;
}

} // namespace Utility
