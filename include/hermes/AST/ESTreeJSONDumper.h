/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
  /// Hide empty fields (empty lists, nullptr, etc) which are rarely populated.
  /// See ignoredEmptyFields_ in the ESTreeJSONDumper class.
  HideEmpty,
  /// Force dumping of all fields regardless of whether they are empty.
  DumpAll,
};

/// Specifies which location information should be dumped by ESTreeJSONDumper.
enum class LocationDumpMode {
  /// Only output locations: line and column.
  Loc,
  /// Only output byte ranges.
  Range,
  /// Output both locations and byte ranges.
  LocAndRange,
};

/// Print out the contents of the given tree to \p os.
/// \p pretty for pretty print the JSON.
/// When \p sm is not null, print the source locations for the AST nodes.
void dumpESTreeJSON(
    llvh::raw_ostream &os,
    ESTree::NodePtr rootNode,
    bool pretty,
    ESTreeDumpMode mode,
    SourceErrorManager *sm = nullptr,
    LocationDumpMode locMode = LocationDumpMode::LocAndRange);

/// Print out the contents of \p rootNode to \p json.
/// Does not call json.endJSONL(), caller should do that if necessary.
/// When \p sm is not null, print the source locations for the AST nodes.
void dumpESTreeJSON(
    JSONEmitter &json,
    ESTree::NodePtr rootNode,
    ESTreeDumpMode mode,
    SourceErrorManager *sm = nullptr,
    LocationDumpMode locMode = LocationDumpMode::LocAndRange);

} // namespace hermes

#endif
