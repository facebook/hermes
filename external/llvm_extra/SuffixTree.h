//===---- SuffixTree.h ------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXTRA_SUFFIXTREE_H
#define LLVM_EXTRA_SUFFIXTREE_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Allocator.h"

#include <vector>

namespace llvm {

/// A node in a suffix tree which represents a substring or suffix.
///
/// Each node has either no children or at least two children, with the root
/// being a exception in the empty tree.
///
/// Children are represented as a map between unsigned integers and nodes. If
/// a node N has a child M on unsigned integer k, then the mapping represented
/// by N is a proper prefix of the mapping represented by M. Note that this,
/// although similar to a trie is somewhat different: each node stores a full
/// substring of the full mapping rather than a single character state.
///
/// Each internal node contains a pointer to the internal node representing
/// the same string, but with the first character chopped off. This is stored
/// in \p Link.
struct SuffixTreeNode {
 public:
  /// Represents an undefined index in the suffix tree.
  static const unsigned EmptyIdx = -1;

  /// The children of this node.
  ///
  /// A child existing on an unsigned integer implies that from the mapping
  /// represented by the current node, there is a way to reach another
  /// mapping by tacking that character on the end of the current string.
  DenseMap<unsigned, SuffixTreeNode *> Children;

  /// The start index of this node's substring in the main string.
  unsigned StartIdx = EmptyIdx;

  /// The end index of this node's substring in the main string (inclusive).
  ///
  /// Every leaf node must have its \p EndIdx incremented at the end of every
  /// step in the construction algorithm. To avoid having to update O(N)
  /// nodes individually at the end of every step, the end index is stored
  /// as a pointer.
  unsigned *EndIdx = nullptr;

  /// For internal nodes, a pointer to the internal node representing
  /// the same sequence with the first character chopped off.
  ///
  /// This acts as a shortcut in Ukkonen's algorithm. One of the things that
  /// Ukkonen's algorithm does to achieve linear-time construction is
  /// keep track of which node the next insert should be at. This makes each
  /// insert O(1), and there are a total of O(N) inserts. The suffix link
  /// helps with inserting children of internal nodes.
  ///
  /// Say we add a child to an internal node with associated mapping S. The
  /// next insertion must be at the node representing S - its first character.
  /// This is given by the way that we iteratively build the tree in Ukkonen's
  /// algorithm. The main idea is to look at the suffixes of each prefix in
  /// the string, starting with the longest suffix of the prefix, and ending
  /// with the shortest. Therefore, if we keep pointers between such nodes, we
  /// can move to the next insertion point in O(1) time. If we don't, then
  /// we'd have to query from the root, which takes O(N) time. This would make
  /// the construction algorithm O(N^2) rather than O(N).
  SuffixTreeNode *Link = nullptr;

  /// The parent of this node. Every node except for the root has a parent.
  SuffixTreeNode *Parent = nullptr;

  /// The length of the string formed by concatenating the edge labels from
  /// the root to this node.
  ///
  /// For leaves, this is the length of the suffix represented by the leaf.
  unsigned ConcatLen = 0;

  /// Returns true if this node is the root of its owning \p SuffixTree.
  bool isRoot() const {
    return StartIdx == EmptyIdx;
  }

  /// Returns true if this node is a leaf (has no children).
  bool isLeaf() const {
    return Children.empty();
  }

  /// Return the number of elements in the substring associated with this node.
  size_t size() const {
    // Is it the root? If so, it's the empty string so return 0.
    if (isRoot())
      return 0;

    assert(*EndIdx != EmptyIdx && "EndIdx is undefined!");

    // Size = the number of elements in the string. Start and end are inclusive.
    return *EndIdx - StartIdx + 1;
  }

  SuffixTreeNode(
      unsigned StartIdx,
      unsigned *EndIdx,
      SuffixTreeNode *Link,
      SuffixTreeNode *Parent)
      : StartIdx(StartIdx), EndIdx(EndIdx), Link(Link), Parent(Parent) {}

  SuffixTreeNode() {}
};

/// A data structure for fast substring queries.
///
/// Suffix trees represent the suffixes of their input strings in their
/// leaves. A suffix tree is a type of compressed trie structure where each
/// node represents an entire substring rather than a single character. Each
/// leaf of the tree is a suffix.
///
/// A suffix tree can be seen as a type of state machine where each state is a
/// substring of the full string. The tree is structured so that, for a string
/// of length N, there are exactly N leaves in the tree. This structure allows
/// us to quickly find repeated substrings of the input string.
///
/// In this implementation, a "string" is a vector of unsigned integers.
/// These integers may result from hashing some data type. A suffix tree can
/// contain 1 or many strings, which can then be queried as one large string.
///
/// The suffix tree is implemented using Ukkonen's algorithm for linear-time
/// suffix tree construction. Ukkonen's algorithm is explained in more detail
/// in the paper by Esko Ukkonen "On-line construction of suffix trees. The
/// paper is available at
///
/// https://www.cs.helsinki.fi/u/ukkonen/SuffixT1withFigs.pdf
class SuffixTree {
 public:
  /// Each element is an integer representing an instruction in the module.
  ArrayRef<unsigned> Str;

  /// Stores internal nodes that have at least two leaf children.
  ///
  /// This is used for finding repeated substrings.
  std::vector<SuffixTreeNode *> MultiLeafParents;

  /// Construct a suffix tree from a sequence of unsigned integers.
  ///
  /// NOTE: The integers will be used as keys in a DenseMap, so \p Str must not
  /// contain the empty key or tombstone key defined in DenseMapInfo<unsigned>.
  ///
  /// \param Str The string to construct the suffix tree for.
  explicit SuffixTree(const std::vector<unsigned> &Str);

 private:
  /// Maintains each node in the tree.
  SpecificBumpPtrAllocator<SuffixTreeNode> NodeAllocator;

  /// The root of the suffix tree.
  ///
  /// The root represents the empty string. It is maintained by the
  /// \p NodeAllocator like every other node in the tree.
  SuffixTreeNode *Root = nullptr;

  /// Maintains the end indices of the internal nodes in the tree.
  ///
  /// Each internal node is guaranteed to never have its end index change
  /// during the construction algorithm; however, leaves must be updated at
  /// every step. Therefore, we need to store leaf end indices by reference
  /// to avoid updating O(N) leaves at every step of construction. Thus,
  /// every internal node must be allocated its own end index.
  BumpPtrAllocator InternalEndIdxAllocator;

  /// The end index of each leaf in the tree.
  unsigned LeafEndIdx = -1;

  /// Helper struct which keeps track of the next insertion point in
  /// Ukkonen's algorithm.
  struct ActiveState {
    /// The next node to insert at.
    SuffixTreeNode *Node;

    /// The index of the first character in the substring currently being added.
    unsigned Idx = SuffixTreeNode::EmptyIdx;

    /// The length of the substring we have to add at the current step.
    unsigned Len = 0;
  };

  /// The point the next insertion will take place at in the
  /// construction algorithm.
  ActiveState Active;

  /// Allocate a leaf node and add it to the tree.
  ///
  /// \param Parent The parent of this node.
  /// \param StartIdx The start index of this node's associated string.
  /// \param Edge The label on the edge leaving \p Parent to this node.
  ///
  /// \returns A pointer to the allocated leaf node.
  SuffixTreeNode *
  insertLeaf(SuffixTreeNode &Parent, unsigned StartIdx, unsigned Edge);

  /// Allocate an internal node and add it to the tree.
  ///
  /// \param Parent The parent of this node. Only null when allocating the
  /// root. \param StartIdx The start index of this node's associated string.
  /// \param EndIdx The end index of this node's associated string.
  /// \param Edge The label on the edge leaving \p Parent to this node.
  ///
  /// \returns A pointer to the allocated internal node.
  SuffixTreeNode *insertInternalNode(
      SuffixTreeNode *Parent,
      unsigned StartIdx,
      unsigned EndIdx,
      unsigned Edge);

  /// Set the ConcatLen property on \p CurrNode and all its descendants. Also
  /// stores nodes in \p MultiLeafParents that have two or more leaf children.
  ///
  /// \param[in] CurrNode The node currently being visited.
  void setConcatLen(SuffixTreeNode &CurrNode);

  /// Construct the suffix tree for the prefix of the input ending at
  /// \p EndIdx.
  ///
  /// Used to construct the full suffix tree iteratively. At the end of each
  /// step, the constructed suffix tree is either a valid suffix tree, or a
  /// suffix tree with implicit suffixes. At the end of the final step, the
  /// suffix tree is a valid tree.
  ///
  /// \param EndIdx The end index of the current prefix in the main string.
  /// \param SuffixesToAdd The number of suffixes that must be added
  /// to complete the suffix tree at the current phase.
  ///
  /// \returns The number of suffixes that have not been added at the end of
  /// this step.
  unsigned extend(unsigned EndIdx, unsigned SuffixesToAdd);
};

} // namespace llvm

#endif // LLVM_EXTRA_SUFFIXTREE_H
