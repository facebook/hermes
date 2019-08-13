/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_HEAPSNAPSHOT_H
#define HERMES_VM_HEAPSNAPSHOT_H

#include "hermes/Public/GCConfig.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/StringSetVector.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/StringRefUtils.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <bitset>
#include <string>

namespace hermes {
namespace vm {

using HeapSizeType = uint32_t;
class StringPrimitive;

/// @name Converters from arbitrary types to string
/// @{
std::string converter(const char *name);
std::string converter(unsigned index);
std::string converter(int index);
std::string converter(const StringPrimitive *str);
std::string converter(UTF16Ref ref);
/// @}

/// A raw dump of the memory as a binary stream.
///
/// It outputs the entire heap, byte for byte, to the given \p os, but first
/// outputs the \p start address to be used as a base for all pointers upon
/// reconstruction.
/// TODO: This format currently does not preserve perfect information, for
///   example NativeFunctionPtr will be randomized on each startup, and cannot
///   be guaranteed to point at the same place.
///   Currently it also does not do anything about string pointers, so those
///   will also be invalid.
///   Something which enables these should be implemented in the future.
void rawHeapSnapshot(llvm::raw_ostream &os, const char *start, const char *end);

class HeapSnapshot {
 public:
  enum class Section : unsigned {
#define V8_SNAPSHOT_SECTION(enumerand, label) enumerand,
#include "hermes/VM/HeapSnapshot.def"
  };

  enum class NodeType : unsigned {
#define V8_NODE_TYPE(enumerand, label) enumerand,
#include "hermes/VM/HeapSnapshot.def"
  };

  enum class EdgeType : unsigned {
#define V8_EDGE_TYPE(enumerand, label) enumerand,
#include "hermes/VM/HeapSnapshot.def"
  };

  using NodeID = uint64_t;
  using NodeIndex = uint32_t;
  using EdgeIndex = uint32_t;

  explicit HeapSnapshot(JSONEmitter &json);

  /// NOTE: this destructor writes to \p json.
  ~HeapSnapshot();

  /// Opens \p section.  All sections between the next section to be closed
  ///(inclusive) and this one (exclusive) will be skipped by implicitly opening
  /// and closing them.
  ///
  /// \pre No other section is already open.
  /// \pre \p section is not the special sentinel, END.
  /// \pre This section has not already been closed (either explicitly, through
  ///     a call to endSection, or implicitly because it was skipped).
  void beginSection(Section section);

  /// Closes \p section.
  ///
  /// \pre \p section is the currently opened section.
  /// \pre \p section is not the END sentinel.
  void endSection(Section section);

  void beginNode();
  void endNode(
      NodeType type,
      llvm::StringRef name,
      NodeID id,
      HeapSizeType selfSize,
      HeapSizeType traceNodeID = 0);

  void addNamedEdge(EdgeType type, llvm::StringRef name, NodeID toNode);
  void addIndexedEdge(EdgeType type, EdgeIndex index, NodeID toNode);

 private:
  void emitMeta();
  void emitStrings();

  /// The next section to be closed.  This class guarantees that all previous
  /// sections will have been written to the JSON emitter.
  Section nextSection_{Section::Nodes};

  /// Whether the nextSection_ has been opened already.
  bool sectionOpened_{false};

  JSONEmitter &json_;
  llvm::DenseMap<NodeID, NodeIndex> nodeToIndex_;
  StringSetVector stringTable_;
  NodeIndex nodeCount_{0};
  HeapSizeType currEdgeCount_{0};
#ifndef NDEBUG
  /// How many edges have currently been added.
  EdgeIndex edgeCount_{0};
  /// How many edges there are expected to be. Used for checking the correctness
  /// of the snapshot.
  EdgeIndex expectedEdges_{0};
#endif
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_HEAPSNAPSHOT_H
