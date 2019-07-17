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
  using StringID = uint32_t;
  struct Node {
    /// The highest-level categorization of the type of an object.
    /// NOTE: These types are chosen to align with v8's types, not what Hermes
    /// actually uses.
    enum class Type {
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

    using ID = uintptr_t;
    using Index = uint32_t;

    Type type;
    StringID name;
    ID id;
    HeapSizeType selfSize;
    HeapSizeType edgeCount;
    HeapSizeType traceNodeId;

    explicit Node(
        Type type,
        StringID name,
        ID id,
        HeapSizeType selfSize,
        HeapSizeType edgeCount,
        HeapSizeType traceNodeId = 0)
        : type(type),
          name(name),
          id(id),
          selfSize(selfSize),
          edgeCount(edgeCount),
          traceNodeId(traceNodeId) {}

    static const char *nodeTypeStr(Type type);
    static Type cellKindToType(CellKind kind);
  };

  struct Edge {
    enum class Type {
      // NOTE: Keep this in sync with the list emitted in endMeta.
      Context,
      Element,
      Property,
      Internal,
      Hidden,
      Shortcut,
      Weak
    };
    using Index = uint32_t;
    // TODO: possibly kill PointerKind in favour of switching on type directly
    enum PointerKind { PKNamed, PKUnnamed };
    struct Named {};
    struct Unnamed {};
    Type type;
    PointerKind pointerKind;
    Node::ID toNode;

    StringID name() const {
      assert(pointerKind == PKNamed);
      return name_;
    }

    Index index() const {
      assert(pointerKind == PKUnnamed);
      return index_;
    }

    explicit Edge(Named tag, Type type, Node::ID toNode, StringID name)
        : type(type), pointerKind(PKNamed), toNode(toNode), name_(name) {}

    explicit Edge(Unnamed tag, Type type, Node::ID toNode, Index index)
        : type(type), pointerKind(PKUnnamed), toNode(toNode), index_(index) {}

   private:
    StringID name_; // valid only if pointerKind == PKNamed
    Index index_; // valid only if pointerKind == PKUnnamed
  };

  explicit V8HeapSnapshot(JSONEmitter &json);

  /// NOTE: this destructor writes to \p json.
  ~V8HeapSnapshot();

  void beginNodes();
  void addNode(Node &&node);
  void endNodes();

  void beginEdges();
  void addEdge(Edge &&edge);
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

  void beginStrings();
  void addString(llvm::StringRef str);
  void endStrings();

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
  llvm::DenseMap<Node::ID, Node::Index> nodeToIndex_;
  Node::Index nodeCount_{0};
#ifndef NDEBUG
  /// How many edges have currently been added.
  Edge::Index edgeCount_{0};
  /// How many edges there are expected to be. Used for checking the correctness
  /// of the snapshot.
  Edge::Index expectedEdges_{0};
#endif

  void beginSection(Section section);
  void endSection(Section section);
  bool didWriteSection(Section section) const;

  static size_t sectionIndex(Section section);
  static uint32_t edgeTypeIndex(Edge::Type);
  static uint32_t nodeTypeIndex(Node::Type);
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_HEAPSNAPSHOT_H
