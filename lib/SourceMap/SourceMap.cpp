/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/SourceMap/SourceMap.h"

using namespace hermes;

namespace hermes {

llvm::Optional<SourceMapTextLocation> SourceMap::getLocationForAddress(
    uint32_t line,
    uint32_t column) const {
  auto seg = this->getSegmentForAddress(line, column);
  // Unmapped location
  if (!seg.hasValue() || !seg->representedLocation.hasValue()) {
    return llvm::None;
  }
  // parseSegment() should have validated this.
  assert(
      (size_t)seg->representedLocation->sourceIndex < sources_.size() &&
      "SourceIndex is out-of-range.");
  std::string fileName =
      getSourceFullPath(seg->representedLocation->sourceIndex);
  return SourceMapTextLocation{
      std::move(fileName),
      (uint32_t)seg->representedLocation->lineIndex + 1,
      (uint32_t)seg->representedLocation->columnIndex + 1};
}

llvm::Optional<SourceMap::Segment> SourceMap::getSegmentForAddress(
    uint32_t line,
    uint32_t column) const {
  if (line == 0 || line > lines_.size()) {
    return llvm::None;
  }

  // line is 1-based.
  uint32_t lineIndex = line - 1;
  auto &segments = lines_[lineIndex];
  if (segments.empty()) {
    return llvm::None;
  }
  assert(column >= 1 && "the column argument to this function is 1-based");
  uint32_t columnIndex = column - 1;
  // Algorithm: we wanted to locate the segment covering
  // the needle(`column`) -- segment.generatedColumn <= column.
  // We achieve it by binary searching the first sentinel
  // segment strictly greater than needle(`column`) and then move backward
  // one slot.
  auto segIter = std::upper_bound(
      segments.begin(),
      segments.end(),
      columnIndex,
      [](uint32_t column, const Segment &seg) {
        return column < (uint32_t)seg.generatedColumn;
      });
  // The found sentinel segment is the first one. No covering segment.
  if (segIter == segments.begin()) {
    return llvm::None;
  }
  // Move back one slot.
  const Segment &target =
      segIter == segments.end() ? segments.back() : *(--segIter);
  return target;
}

} // namespace hermes
