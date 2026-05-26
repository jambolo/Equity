//! Bitcoin Merkle tree — array-backed binary tree of SHA-256d hashes with the
//! Bitcoin-specific oddity that odd-count layers duplicate their last node.
//!
//! Layout:
//!   * `tree[1]` is the root (index 0 unused),
//!   * leaves live at `tree[offset..offset + padded_leaves]`,
//!   * `offset` is the smallest power of two >= `padded_leaves`.

use crypto::sha256;

pub const HASH_SIZE: usize = 32;
pub type Hash = [u8; HASH_SIZE];

const ROOT: usize = 1;

pub struct MerkleTree {
    tree: Vec<Hash>,
    n_leaves: usize,
    offset: usize,
}

impl MerkleTree {
    pub fn new(hashes: &[Hash]) -> Self {
        let n_leaves = hashes.len();

        if n_leaves == 0 {
            return Self {
                tree: Vec::new(),
                n_leaves,
                offset: 0,
            };
        }

        if n_leaves == 1 {
            // Odd Bitcoin convention: single-leaf tree's root *is* the leaf.
            let mut tree = vec![[0u8; HASH_SIZE]; 1];
            tree.push(hashes[0]);
            return Self {
                tree,
                n_leaves,
                offset: 1,
            };
        }

        // Round leaf count up to even.
        let padded = (n_leaves + 1) & !1;
        let offset = padded.next_power_of_two();

        let mut tree = vec![[0u8; HASH_SIZE]; offset];
        tree.extend_from_slice(hashes);
        if !n_leaves.is_multiple_of(2) {
            tree.push(*tree.last().unwrap());
        }

        let mut n = padded / 2;
        let mut first = offset / 2;
        while n > 0 {
            for i in first..(first + n) {
                let left = tree[left_child(i)];
                let right = tree[right_child(i)];
                tree[i] = sha256::double_sha256(&concat(&left, &right));
            }
            if !n.is_multiple_of(2) && first > ROOT {
                tree.push([0u8; HASH_SIZE]);
                let idx = first + n;
                if idx >= tree.len() {
                    tree.resize(idx + 1, [0u8; HASH_SIZE]);
                }
                tree[idx] = tree[idx - 1];
                n += 1;
            }
            n /= 2;
            first /= 2;
        }

        Self {
            tree,
            n_leaves,
            offset,
        }
    }

    pub fn root(&self) -> Hash {
        if self.tree.len() <= ROOT {
            return [0u8; HASH_SIZE];
        }
        self.tree[ROOT]
    }

    pub fn hash_at(&self, i: usize) -> Hash {
        if i >= self.n_leaves {
            return [0u8; HASH_SIZE];
        }
        self.tree[self.offset + i]
    }

    pub fn proof(&self, mut i: usize) -> Vec<Hash> {
        if i >= self.n_leaves {
            return Vec::new();
        }
        i += self.offset;
        let mut p = Vec::new();
        while i > ROOT {
            p.push(self.tree[i ^ 1]);
            i /= 2;
        }
        p
    }

    pub fn verify(hash: &Hash, mut i: usize, proof: &[Hash], root: &Hash) -> bool {
        let mut result = *hash;
        for sibling in proof {
            let concatenated = if i.is_multiple_of(2) {
                concat(&result, sibling)
            } else {
                concat(sibling, &result)
            };
            result = sha256::double_sha256(&concatenated);
            i /= 2;
        }
        result == *root
    }
}

fn left_child(i: usize) -> usize {
    i * 2
}

fn right_child(i: usize) -> usize {
    i * 2 + 1
}

fn concat(left: &Hash, right: &Hash) -> [u8; HASH_SIZE * 2] {
    let mut out = [0u8; HASH_SIZE * 2];
    out[..HASH_SIZE].copy_from_slice(left);
    out[HASH_SIZE..].copy_from_slice(right);
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    fn hash_of(v: u8) -> Hash {
        [v; HASH_SIZE]
    }

    #[test]
    fn test_single_leaf_tree_root_is_leaf() {
        let leaf = hash_of(1);
        let t = MerkleTree::new(&[leaf]);
        assert_eq!(t.root(), leaf);
    }

    #[test]
    fn test_proof_round_trip_balanced() {
        let leaves: Vec<Hash> = (1..=4u8).map(hash_of).collect();
        let t = MerkleTree::new(&leaves);
        let root = t.root();
        for (i, h) in leaves.iter().enumerate() {
            let p = t.proof(i);
            assert!(MerkleTree::verify(h, i, &p, &root), "leaf {i}");
        }
    }

    #[test]
    fn test_proof_round_trip_odd_leaf_count() {
        let leaves: Vec<Hash> = (1..=3u8).map(hash_of).collect();
        let t = MerkleTree::new(&leaves);
        let root = t.root();
        for (i, h) in leaves.iter().enumerate() {
            let p = t.proof(i);
            assert!(MerkleTree::verify(h, i, &p, &root), "leaf {i}");
        }
    }

    #[test]
    fn test_proof_fails_for_wrong_index() {
        let leaves: Vec<Hash> = (1..=4u8).map(hash_of).collect();
        let t = MerkleTree::new(&leaves);
        let root = t.root();
        let proof = t.proof(0);
        assert!(!MerkleTree::verify(&leaves[1], 0, &proof, &root));
    }

    #[test]
    fn test_hash_at() {
        let leaves: Vec<Hash> = (1..=4u8).map(hash_of).collect();
        let t = MerkleTree::new(&leaves);
        for (i, h) in leaves.iter().enumerate() {
            assert_eq!(t.hash_at(i), *h);
        }
        // Out of range
        assert_eq!(t.hash_at(99), [0u8; HASH_SIZE]);
    }

    #[test]
    fn test_empty_tree_has_zero_root() {
        let t = MerkleTree::new(&[]);
        assert_eq!(t.root(), [0u8; HASH_SIZE]);
        assert!(t.proof(0).is_empty());
    }

    #[test]
    fn test_two_leaves_root_is_concat_hash() {
        let a = hash_of(1);
        let b = hash_of(2);
        let t = MerkleTree::new(&[a, b]);
        let mut concat = [0u8; HASH_SIZE * 2];
        concat[..HASH_SIZE].copy_from_slice(&a);
        concat[HASH_SIZE..].copy_from_slice(&b);
        assert_eq!(t.root(), crypto::sha256::double_sha256(&concat));
    }

    #[test]
    fn test_proof_fails_with_wrong_root() {
        let leaves: Vec<Hash> = (1..=4u8).map(hash_of).collect();
        let t = MerkleTree::new(&leaves);
        let mut bogus = t.root();
        bogus[0] ^= 0xFF;
        let proof = t.proof(0);
        assert!(!MerkleTree::verify(&leaves[0], 0, &proof, &bogus));
    }

    proptest::proptest! {
        #[test]
        fn prop_merkle_proof_round_trip(
            n in 1usize..16,
            seed in any::<u8>(),
        ) {
            let leaves: Vec<Hash> = (0..n).map(|i| hash_of(seed.wrapping_add(i as u8))).collect();
            let t = MerkleTree::new(&leaves);
            let root = t.root();
            for (i, h) in leaves.iter().enumerate() {
                let p = t.proof(i);
                proptest::prop_assert!(MerkleTree::verify(h, i, &p, &root), "leaf {} of {}", i, n);
            }
        }
    }

    use proptest::prelude::any;
}
