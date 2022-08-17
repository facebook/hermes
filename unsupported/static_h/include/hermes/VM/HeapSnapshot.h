/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_HEAPSNAPSHOT_H
#define HERMES_VM_HEAPSNAPSHOT_H

#include "hermes/Public/DebuggerTypes.h"
#include "hermes/Public/GCConfig.h"
#include "hermes/Support/JSONEmitter.h"
#include "hermes/Support/StringSetVector.h"
#include "hermes/VM/CellKind.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/StackTracesTree-NoRuntime.h"
#include "hermes/VM/StringRefUtils.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/Optional.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/raw_ostream.h"

#include <bitset>
#include <chrono>
#include <string>

namespace hermes {
namespace vm {

using HeapSizeType = uint32_t;
class StringPrimitive;
struct StackTracesTree;

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
void rawHeapSnapshot(llvh::raw_ostream &os, const char *start, const char *end);

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

  // The number of declared fields plus one for the type field.
  static constexpr uint32_t V8_SNAPSHOT_NODE_FIELD_COUNT = 1
#define V8_NODE_FIELD(label, type) +1
#include "hermes/VM/HeapSnapshot.def"
      ;

  // The number of declared fields plus one for the type field.
  static constexpr uint32_t V8_SNAPSHOT_EDGE_FIELD_COUNT = 1
#define V8_EDGE_FIELD(label, type) +1
#include "hermes/VM/HeapSnapshot.def"
      ;

  static constexpr uint32_t V8_SNAPSHOT_LOCATION_FIELD_COUNT = 0
#define V8_LOCATION_FIELD(label) +1
#include "hermes/VM/HeapSnapshot.def"
      ;

  static constexpr uint32_t V8_SNAPSHOT_SAMPLE_FIELD_COUNT = 0
#define V8_SAMPLE_FIELD(label) +1
#include "hermes/VM/HeapSnapshot.def"
      ;

  using NodeID = uint32_t;
  using NodeIndex = uint32_t;
  using EdgeIndex = uint32_t;

#ifdef HERMES_MEMORY_INSTRUMENTATION
  HeapSnapshot(JSONEmitter &json, StackTracesTree *stackTracesTree);

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
      llvh::StringRef name,
      NodeID id,
      HeapSizeType selfSize,
      HeapSizeType traceNodeID);

  void addNamedEdge(EdgeType type, llvh::StringRef name, NodeID toNode);
  void addIndexedEdge(EdgeType type, EdgeIndex index, NodeID toNode);

  /// Adds a location for the given node. This will identify a source location
  /// for a node when it is viewed in a heap snapshot visualizer.
  /// This is not a stack trace where the object was allocated, but a static
  /// location. For example, functions use their source location as their
  /// location. User-defined objects have the constructor function.
  /// NOTE: If a script ID is not available (for example if the debugger isn't
  ///   on), don't call this function.
  /// \param id The object to attach a location to.
  /// \param script The ID of the script in which this location resides.
  /// \param line The 1-based line in \p script.
  /// \param column The 1-based column in \p script.
  void addLocation(
      NodeID id,
      ::facebook::hermes::debugger::ScriptID script,
      uint32_t line,
      uint32_t column);

  void addSample(std::chrono::microseconds timestamp, NodeID lastSeenObjectID);

  static const char *nodeTypeToName(NodeType type);

  static const char *edgeTypeToName(EdgeType type);

  void emitAllocationTraceInfo();

 private:
  void emitMeta();
  size_t countFunctionTraceInfos();
  void emitStrings();

  /// The next section to be closed.  This class guarantees that all
  /// previous sections will have been written to the JSON emitter.
  Section nextSection_{Section::Nodes};

  /// Whether the nextSection_ has been opened already.
  bool sectionOpened_{false};

  JSONEmitter &json_;
  StackTracesTree *stackTracesTree_;
  llvh::DenseMap<NodeID, NodeIndex> nodeToIndex_;
  std::shared_ptr<StringSetVector> stringTable_;
  NodeIndex nodeCount_{0};
  HeapSizeType currEdgeCount_{0};
  struct TraceNodeStats {
    HeapSizeType count{0};
    HeapSizeType size{0};
  };
  llvh::DenseMap<HeapSizeType, TraceNodeStats> traceNodeStats_;

#ifndef NDEBUG
  /// How many edges have currently been added.
  EdgeIndex edgeCount_{0};
  /// How many edges there are expected to be. Used for checking the
  /// correctness of the snapshot.
  EdgeIndex expectedEdges_{0};
#endif

#endif // HERMES_MEMORY_INSTRUMENTATION
};

#ifdef HERMES_MEMORY_INSTRUMENTATION
/// Use this class to output the Chrome .heapprofile file extension type.
/// It's a JSON-based output, here's a small example of the top of the file:
/// \code
/// {
///     "head": {
///         "callFrame": {
///             "functionName": "(root)",
///             "scriptId": "0",
///             "url": "",
///             "lineNumber": -1,
///             "columnNumber": -1
///         },
///         "selfSize": 0,
///         "id": 1,
///         "children": [
///             {
///                 "callFrame": {
///                     "functionName": "foo",
///                     "scriptId": "5",
///                     "url": "file:///test_page.html",
///                     "lineNumber": 18,
///                     "columnNumber": 69
///                 },
///                 "selfSize": 196664,
///                 "id": 3,
///                 "children": [...]
///             },
///             ...
///         ]
///     },
///     "samples": [
///         {
///             "size": 16432,
///             "nodeId": 9,
///             "ordinal": 23
///         },
///         ...
///         {
///             "size": 131080,
///             "nodeId": 4,
///             "ordinal": 2
///         }
///     ],
/// }
/// \endcode
/// Some things to note about the format:
/// * Chrome only stores the declaration of each function in a stack frame, not
///   which lines called each other, or which line allocated an object.
/// * selfSize in each frame denotes how many bytes are allocated in that
///   function
/// * lineNumber and columnNumber are 0-based, whereas most editors display line
///   numbers starting at 1.
/// * In the samples section, each sample tuple is the size of the heap, the id
///   of the stack frame node described in the "head" section. "ordinal"
///   describes the chronological order of samples. Samples are not placed in
///   the JSON format in the order in which they occurred, but in an arbitrary
///   order based on the call graph. "ordinal" is used to re-order them into a
///   timeline of when allocations happened.
/// * Each sample internally tracks the count of how many allocations of a given
///   size happened. This is then translated to an estimate of the true number
///   of allocations that occurred at a spot, based on the sampling process.
/// * When a sampled allocation is deleted, the count and size are decremented.
///   this process is meant to track only memory that is still alive at the time
///   that the profile is reported back to the user. This means if a function
///   creates a lot of garbage, that will not be reported by this profile.
/// This format is much less optimized than the Chrome heap snapshot or heap
/// profile format, and uses very basic JSON. It is also a completely separate
/// format from a heap snapshot, hence why it's a different class. It re-uses a
/// StackTracesTree to share information with normal heap profiles.
class ChromeSamplingMemoryProfile final {
 public:
  explicit ChromeSamplingMemoryProfile(JSONEmitter &json);
  ~ChromeSamplingMemoryProfile();

  void emitTree(
      StackTracesTree *stackTracesTree,
      const llvh::DenseMap<
          StackTracesTreeNode *,
          llvh::DenseMap<size_t, size_t>> &sizesToCounts);
  void beginSamples();
  void emitSample(size_t size, StackTracesTreeNode *node, uint64_t id);
  void endSamples();

 private:
  JSONEmitter &json_;

  void emitNode(
      StackTracesTreeNode *node,
      StringSetVector &strings,
      const llvh::DenseMap<
          StackTracesTreeNode *,
          llvh::DenseMap<size_t, size_t>> &sizesToCounts);
};

#endif // HERMES_MEMORY_INSTRUMENTATION

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_HEAPSNAPSHOT_H
