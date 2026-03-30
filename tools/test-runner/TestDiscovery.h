/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_TESTRUNNER_TESTDISCOVERY_H
#define HERMES_TOOLS_TESTRUNNER_TESTDISCOVERY_H

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/raw_ostream.h"

#include <algorithm>
#include <string>
#include <vector>

namespace hermes {
namespace testrunner {

/// Supported test suite kinds.
enum class SuiteKind {
  Test262,
  Mjsunit,
  CVEs,
  Esprima,
  Flow,
};

/// Short display name for a suite kind.
inline const char *suiteKindName(SuiteKind kind) {
  switch (kind) {
    case SuiteKind::Test262:
      return "test262";
    case SuiteKind::Mjsunit:
      return "mjsunit";
    case SuiteKind::CVEs:
      return "CVEs";
    case SuiteKind::Esprima:
      return "esprima";
    case SuiteKind::Flow:
      return "flow";
  }
  return "unknown";
}

/// A discovered test file with its suite classification.
struct TestEntry {
  /// Absolute path to the .js test file.
  std::string path;
  /// Which test suite this file belongs to.
  SuiteKind suiteKind;
  /// Root directory of the suite (e.g., "/path/to/test262/").
  std::string suiteDir;
  /// Human-readable test name: "{suite} :: {relative_path}".
  std::string fullName;
};

/// Detect the suite kind from a file path by searching for known directory
/// markers. Returns true and sets kind/suiteDir if found.
inline bool
detectSuite(llvh::StringRef path, SuiteKind &kind, std::string &suiteDir) {
  struct Marker {
    const char *name;
    SuiteKind kind;
  };
  // Order matters: "flow/" must use trailing slash to avoid matching
  // "flowtest/".
  static const Marker markers[] = {
      {"test262/", SuiteKind::Test262},
      {"mjsunit/", SuiteKind::Mjsunit},
      {"CVEs/", SuiteKind::CVEs},
      {"esprima/", SuiteKind::Esprima},
      {"flow/", SuiteKind::Flow},
  };

  for (const auto &m : markers) {
    // Use rfind to pick the innermost match for repeated folder names.
    auto pos = path.rfind(m.name);
    if (pos != llvh::StringRef::npos) {
      kind = m.kind;
      suiteDir = path.substr(0, pos + strlen(m.name)).str();
      return true;
    }
  }
  return false;
}

/// Check whether a file path is a valid test file.
/// Must end with .js and must not be a test262 fixture (*_FIXTURE.js).
inline bool isTestFile(llvh::StringRef path) {
  if (llvh::sys::path::extension(path) != ".js")
    return false;

  // test262 fixture files are helpers, not tests.
  if (path.endswith("_FIXTURE.js") && path.contains("test262"))
    return false;

  return true;
}

/// Create a TestEntry from a file path and its detected suite info.
inline TestEntry makeTestEntry(
    llvh::StringRef path,
    SuiteKind kind,
    const std::string &suiteDir) {
  llvh::StringRef rel = path;
  if (rel.startswith(suiteDir))
    rel = rel.drop_front(suiteDir.size());
  std::string fullName = std::string(suiteKindName(kind)) + " :: " + rel.str();
  return {path.str(), kind, suiteDir, fullName};
}

/// Recursively collect .js test files from a directory.
inline void collectTestFiles(
    const llvh::Twine &dirPath,
    std::vector<TestEntry> &out) {
  std::error_code ec;
  for (llvh::sys::fs::recursive_directory_iterator it(dirPath, ec), end;
       it != end;
       it.increment(ec)) {
    if (ec) {
      llvh::errs() << "Error traversing " << dirPath << ": " << ec.message()
                   << "\n";
      break;
    }
    llvh::StringRef entry = it->path();
    if (!isTestFile(entry))
      continue;

    SuiteKind kind;
    std::string suiteDir;
    if (!detectSuite(entry, kind, suiteDir))
      continue;

    out.push_back(makeTestEntry(entry, kind, suiteDir));
  }
}

/// Discover all test files under the given paths.
/// Each path can be a file or directory. Directories are walked recursively.
/// Returns test entries sorted by path for deterministic ordering.
inline std::vector<TestEntry> discoverTests(
    const std::vector<std::string> &paths) {
  std::vector<TestEntry> entries;

  for (const auto &p : paths) {
    llvh::sys::fs::file_status status;
    if (llvh::sys::fs::status(p, status)) {
      llvh::errs() << "Warning: cannot stat '" << p << "', skipping.\n";
      continue;
    }
    if (llvh::sys::fs::is_directory(status)) {
      collectTestFiles(p, entries);
    } else if (llvh::sys::fs::is_regular_file(status)) {
      llvh::StringRef path(p);
      if (!isTestFile(path))
        continue;

      SuiteKind kind;
      std::string suiteDir;
      if (!detectSuite(path, kind, suiteDir))
        continue;

      entries.push_back(makeTestEntry(path, kind, suiteDir));
    } else {
      llvh::errs() << "Warning: '" << p
                   << "' is not a file or directory, skipping.\n";
    }
  }

  // Sort for deterministic test ordering.
  std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b) {
    return a.path < b.path;
  });
  return entries;
}

} // namespace testrunner
} // namespace hermes

#endif // HERMES_TOOLS_TESTRUNNER_TESTDISCOVERY_H
