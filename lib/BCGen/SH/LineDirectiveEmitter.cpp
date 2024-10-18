/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LineDirectiveEmitter.h"

namespace hermes::sh {

void LineDirectiveEmitter::write_impl(const char *ptr, size_t size) {
  if (enabled_) {
    // We write the given bytes in newline-delimited chunks, with line
    // directives potentially separating the chunks. This is the beginning of
    // the current chunk we are going to write.
    const char *chunkBegin = ptr;
    bool seenNewline = endedOnNewline_;
    for (const char *cur = ptr, *e = ptr + size; cur < e; ++cur) {
      // If we see a newline, commit the current chunk, including the newline.
      if (*cur == '\n') {
        seenNewline = true;
        os_.write(chunkBegin, (cur - chunkBegin) + 1);
        chunkBegin = cur + 1;
      } else if (seenNewline) {
        // This is the first non-newline character after we have seen a
        // newline. Insert the line directive.
        writeLineDirective();
        seenNewline = false;
      }
    }
    // Write whatever remaining characters there are.
    os_.write(chunkBegin, (ptr + size) - chunkBegin);
  } else {
    os_.write(ptr, size);
  }
  endedOnNewline_ = size && ptr[size - 1] == '\n';
}

void LineDirectiveEmitter::writeLineDirective() {
  os_ << "#line " << info_.first << " \"";
  os_.write_escaped(info_.second) << "\"\n";
}

void LineDirectiveEmitter::setDirectiveInfo(
    SMLoc loc,
    SourceErrorManager &srcMgr) {
  if (loc.isValid()) {
    hermes::SourceErrorManager::SourceCoords coords;
    if (srcMgr.findBufferLineAndLoc(loc, coords)) {
      llvh::StringRef filename = srcMgr.getSourceUrl(coords.bufId);
      LineDirectiveInfo newInfo = {coords.line, filename};
      if (newInfo != info_) {
        flush();
        info_ = newInfo;
      }
    }
  }
}

void LineDirectiveEmitter::enableLineDirectives() {
  flush();
  enabled_ = true;
}

void LineDirectiveEmitter::disableLineDirectives() {
  flush();
  enabled_ = false;
  // Reset the line directive information when we disable. We shouldn't carry
  // over line info between uses of the emitter.
  info_ = LineDirectiveInfo{};
}

} // namespace hermes::sh
