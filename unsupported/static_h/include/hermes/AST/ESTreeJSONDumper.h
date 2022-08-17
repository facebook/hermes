/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_ESTREEJSONDUMPER_H
#define HERMES_AST_ESTREEJSONDUMPER_H

#include "hermes/AST/ESTree.h"

#include "llvh/Support/raw_ostream.h"

namespace hermes {

class JSONEmitter;

/// Specifies which fields should be dumped by ESTreeJSONDumper.
enum class ESTreeDumpMode {
  /// Hide every empty field, regardless of how common it is.
  Compact,
  /// Hide empty fields (empty lists, nullptr, etc) which are rarely populated.
  /// See ignoredEmptyFields_ in the ESTreeJSONDumper class.
  HideEmpty,
  /// Force dumping of all fields regardless of whether they are empty.
  DumpAll,
};

/// Specifies which location information should be dumped by ESTreeJSONDumper.
enum class LocationDumpMode {
  /// Dump no locations.
  None,
  /// Only output locations: line and column.
  Loc,
  /// Only output byte ranges.
  Range,
  /// Output both locations and byte ranges.
  LocAndRange,
};

/// Specifies whether to include the "raw" property where available.
enum class ESTreeRawProp { Exclude, Include };

/// Print the contents of \p rng to \p json using offsets computed relative to
/// \p buffer. Range is printed as an 2-array [start, end]
void dumpSMRangeJSON(
    JSONEmitter &json,
    llvh::SMRange rng,
    const llvh::MemoryBuffer *buffer);

/// Print out the contents of \p rootNode to \p json without locations.
void dumpESTreeJSON(
    llvh::raw_ostream &os,
    ESTree::NodePtr rootNode,
    bool pretty,
    ESTreeDumpMode mode);

/// Print out the contents of the given tree to \p os.
/// \p pretty for pretty print the JSON.
/// When \p sm is not null, print the source locations for the AST nodes.
void dumpESTreeJSON(
    llvh::raw_ostream &os,
    ESTree::NodePtr rootNode,
    bool pretty,
    ESTreeDumpMode mode,
    SourceErrorManager &sm,
    LocationDumpMode locMode,
    ESTreeRawProp rawProp = ESTreeRawProp::Include);

/// Print out the contents of \p rootNode to \p json.
/// Does not call json.endJSONL(), caller should do that if necessary.
/// \p locMode how to print the source locations for the AST nodes.
/// \p includeSourceLocs if non-null, only perform source loc printing on
///   elements of includeSourceLocs. If null, it is ignored.
void dumpESTreeJSON(
    JSONEmitter &json,
    ESTree::NodePtr rootNode,
    ESTreeDumpMode mode,
    SourceErrorManager &sm,
    LocationDumpMode locMode,
    const ESTree::NodeKindSet *includeSourceLocs,
    ESTreeRawProp rawProp = ESTreeRawProp::Include);

} // namespace hermes

#endif
