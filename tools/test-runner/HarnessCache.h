/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_TESTRUNNER_HARNESSCACHE_H
#define HERMES_TOOLS_TESTRUNNER_HARNESSCACHE_H

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/raw_ostream.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace hermes {
namespace testrunner {

/// Cached harness file content for test262.
///
/// Loads all .js files from the test262/harness/ directory once at startup
/// so that test source assembly never touches the filesystem again.
class HarnessCache {
  /// All .js files from test262/harness/, keyed by filename
  /// (e.g. "sta.js", "assert.js", "sm/shell.js").
  std::unordered_map<std::string, std::string> files_;

 public:
  HarnessCache() = default;

  /// Load all .js harness files from the given test262 harness directory.
  /// Returns true on success, false if the directory cannot be read.
  bool load(llvh::StringRef harnessDir) {
    std::error_code ec;
    for (llvh::sys::fs::recursive_directory_iterator it(harnessDir, ec), end;
         it != end;
         it.increment(ec)) {
      if (ec) {
        llvh::errs() << "Error reading harness dir: " << ec.message() << "\n";
        return false;
      }

      llvh::StringRef path = it->path();
      if (llvh::sys::path::extension(path) != ".js")
        continue;

      auto fileBuf = llvh::MemoryBuffer::getFile(path);
      if (!fileBuf) {
        llvh::errs() << "Warning: cannot read harness file '" << path << "'\n";
        continue;
      }

      // Key is the relative path from the harness directory.
      llvh::StringRef rel = path;
      if (rel.startswith(harnessDir)) {
        rel = rel.drop_front(harnessDir.size());
        // Strip leading path separator.
        if (!rel.empty() && (rel[0] == '/' || rel[0] == '\\'))
          rel = rel.drop_front(1);
      }

      files_[rel.str()] = (*fileBuf)->getBuffer().str();
    }

    return true;
  }

  /// Look up a harness file by name. Returns nullptr if not found.
  const std::string *get(llvh::StringRef name) const {
    auto it = files_.find(name.str());
    if (it == files_.end())
      return nullptr;
    return &it->second;
  }

  /// Number of loaded harness files.
  size_t size() const {
    return files_.size();
  }

  /// Assemble a complete test source from harness includes and test code.
  ///
  /// \p includes - harness filenames to prepend (from frontmatter).
  /// \p testSrc - the raw test source (frontmatter already stripped).
  /// \p isStrict - whether to prepend 'use strict';.
  ///
  /// Returns the assembled source string.
  std::string buildSource(
      const std::vector<std::string> &includes,
      llvh::StringRef testSrc,
      bool isStrict) const {
    std::string result;

    // Estimate capacity.
    size_t capacity = testSrc.size();
    if (isStrict)
      capacity += 15; // "'use strict';\n"
    for (const auto &inc : includes) {
      if (auto *content = get(inc))
        capacity += content->size() + 1;
    }
    result.reserve(capacity);

    if (isStrict)
      result += "'use strict';\n";

    for (const auto &inc : includes) {
      if (auto *content = get(inc)) {
        result += *content;
        result += '\n';
      } else {
        llvh::errs() << "Warning: harness file '" << inc << "' not found\n";
      }
    }

    result += testSrc.str();
    return result;
  }
};

} // namespace testrunner
} // namespace hermes

#endif // HERMES_TOOLS_TESTRUNNER_HARNESSCACHE_H
