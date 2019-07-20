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

/// Escapes the control characters in a UTF-8 string. Used for embedding an
/// arbitrary string into JSON.
std::string escapeJSON(llvm::StringRef s);
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

class V8HeapSnapshot {
 public:
  /// The highest-level categorization of the type of an object.
  /// NOTE: These types are chosen to align with v8's types, not what Hermes
  /// actually uses.
  enum class NodeType : unsigned {
    Hidden,
    Array,
    String,
    Object,
    Code,
    Closure,
    Regexp,
    Number,
    Native,
    // Synthetic means it's not shown to the user, but only exists to meet the
    // requirements of a graph (for example, the GC roots are synthetic).
    Synthetic,
    ConcatenatedString,
    SlicedString,
    Symbol,
    BigInt,
    NumTypes,
  };

  enum class EdgeType : unsigned {
    // NOTE: Keep this in sync with the list emitted in endMeta.
    Context,
    Element,
    Property,
    Internal,
    Hidden,
    Shortcut,
    Weak
  };

  using NodeID = uintptr_t;
  using NodeIndex = uint32_t;
  using EdgeIndex = uint32_t;

  static NodeType cellKindToNodeType(CellKind kind);

  explicit V8HeapSnapshot(JSONEmitter &json);

  /// NOTE: this destructor writes to \p json.
  ~V8HeapSnapshot();

  void beginNodes();
  void addNode(
      NodeType type,
      llvm::StringRef name,
      NodeID id,
      HeapSizeType selfSize,
      HeapSizeType edgeCount,
      HeapSizeType traceNodeID = 0);
  void endNodes();

  void beginEdges();

  void addNamedEdge(EdgeType type, llvm::StringRef name, NodeID toNode);
  void addIndexedEdge(EdgeType type, EdgeIndex index, NodeID toNode);

  void endEdges();

  void emitMeta();

  void beginTraceFunctionInfos();
  void endTraceFunctionInfos();

  void beginTraceTree();
  void endTraceTree();

  void beginSamples();
  void endSamples();

  void beginLocations();
  void endLocations();

  void emitStrings();

 private:
  enum class Section {
    Nodes,
    Edges,
    TraceFunctionInfos,
    TraceTree,
    Samples,
    Locations,
    Strings
  };

  JSONEmitter &json_;
  std::bitset<8> sectionsWritten_;
  llvm::Optional<Section> currentSection_;
  llvm::DenseMap<NodeID, NodeIndex> nodeToIndex_;
  StringSetVector stringTable_;
  NodeIndex nodeCount_{0};
#ifndef NDEBUG
  /// How many edges have currently been added.
  EdgeIndex edgeCount_{0};
  /// How many edges there are expected to be. Used for checking the correctness
  /// of the snapshot.
  EdgeIndex expectedEdges_{0};
#endif

  void beginSection(Section section);
  void endSection(Section section);
  bool didWriteSection(Section section) const;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_HEAPSNAPSHOT_H
