/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_TESTRUNNER_FRONTMATTER_H
#define HERMES_TOOLS_TESTRUNNER_FRONTMATTER_H

#include "llvh/ADT/StringRef.h"

#include <string>
#include <vector>

namespace hermes {
namespace testrunner {

/// Expected failure specification from the `negative` frontmatter key.
struct NegativeExpectation {
  /// Phase at which the error is expected: "parse", "resolution", or "runtime".
  std::string phase;
  /// The expected error constructor name (e.g. "SyntaxError").
  std::string errorType;
};

/// Parsed test262 frontmatter record.
struct TestRecord {
  std::string description;
  std::vector<std::string> flags;
  std::vector<std::string> features;
  std::vector<std::string> includes;
  NegativeExpectation negative; // empty phase means no negative expectation
  std::string src; // test source with frontmatter stripped

  bool hasFlag(llvh::StringRef flag) const {
    for (const auto &f : flags)
      if (f == flag)
        return true;
    return false;
  }

  bool isModule() const {
    return hasFlag("module");
  }
  bool isAsync() const {
    return hasFlag("async");
  }
  bool isRaw() const {
    return hasFlag("raw");
  }
  bool isNoStrict() const {
    return hasFlag("noStrict");
  }
  bool isOnlyStrict() const {
    return hasFlag("onlyStrict");
  }
  bool hasNegative() const {
    return !negative.phase.empty();
  }
};

/// Parse a YAML value that is a list written as either:
///   - item1\n  - item2\n  (block style)
///   [item1, item2]         (flow/inline style)
inline std::vector<std::string> parseYamlList(llvh::StringRef value) {
  std::vector<std::string> result;
  auto trimmed = value.trim();

  // Flow style: [item1, item2]
  if (trimmed.startswith("[") && trimmed.endswith("]")) {
    auto inner = trimmed.drop_front(1).drop_back(1).trim();
    while (!inner.empty()) {
      auto [item, rest] = inner.split(',');
      auto t = item.trim();
      if (!t.empty())
        result.push_back(t.str());
      inner = rest.trim();
      if (rest.empty())
        break;
    }
    return result;
  }

  // Block style: lines starting with "  - "
  llvh::SmallVector<llvh::StringRef, 8> lines;
  value.split(lines, '\n');
  for (auto line : lines) {
    auto t = line.trim();
    if (t.startswith("- ")) {
      result.push_back(t.drop_front(2).trim().str());
    } else if (t.startswith("-") && t.size() == 1) {
      // bare "- " with nothing after
    }
  }
  return result;
}

/// Strip a leading license/copyright comment block from source text.
/// Returns the source with the license removed.
inline std::string stripLicenseHeader(llvh::StringRef src) {
  if (!src.contains("Copyright") && !src.contains("Public Domain")) {
    return src.str();
  }
  if (!src.contains("All rights reserved") && !src.contains("Public Domain")) {
    return src.str();
  }

  llvh::SmallVector<llvh::StringRef, 16> srcLines;
  src.split(srcLines, '\n');
  size_t pos = 0;
  size_t licenseEnd = 0;
  bool inLicense = false;
  for (auto line : srcLines) {
    auto t = line.trim();
    if (!inLicense && (t.startswith("// Copyright") || t.startswith("/*"))) {
      if (t.contains("Copyright") || t.contains("Public Domain")) {
        inLicense = true;
        licenseEnd = pos + line.size() + 1; // +1 for newline
      }
    } else if (inLicense) {
      if (t.startswith("//") || t.startswith("*") || t.endswith("*/")) {
        licenseEnd = pos + line.size() + 1;
        if (t.endswith("*/"))
          inLicense = false;
      } else {
        break;
      }
    }
    pos += line.size() + 1;
  }

  if (licenseEnd > 0 && licenseEnd <= src.size())
    return src.substr(licenseEnd).str();
  return src.str();
}

/// Parse a test262 source file's YAML frontmatter.
///
/// Extracts the YAML between /*--- and ---*/ markers.
/// Uses a simplified YAML parser sufficient for test262 frontmatter.
inline TestRecord parseFrontmatter(llvh::StringRef content) {
  TestRecord record;

  // Find frontmatter markers.
  auto startPos = content.find("/*---");
  if (startPos == llvh::StringRef::npos) {
    record.src = content.str();
    return record;
  }

  auto afterStart = startPos + 5; // "/*---" is 5 chars
  auto endPos = content.find("---*/", afterStart);
  if (endPos == llvh::StringRef::npos) {
    record.src = content.str();
    return record;
  }

  auto afterEnd = endPos + 5; // "---*/" is 5 chars

  // Extract YAML content.
  auto yaml = content.slice(afterStart, endPos).trim();

  // Strip frontmatter block from source (including trailing blank lines).
  auto blockEnd = afterEnd;
  auto remaining = content.substr(blockEnd);
  while (!remaining.empty()) {
    auto nl = remaining.find('\n');
    llvh::StringRef line;
    if (nl == llvh::StringRef::npos) {
      line = remaining;
    } else {
      line = remaining.substr(0, nl);
    }
    if (line.trim().empty()) {
      blockEnd += line.size();
      if (nl != llvh::StringRef::npos)
        blockEnd += 1; // newline
      remaining = content.substr(blockEnd);
    } else {
      break;
    }
  }

  // Build source: everything before frontmatter + everything after.
  std::string src =
      content.substr(0, startPos).str() + content.substr(blockEnd).str();

  // Parse simplified YAML first, so we know if this is a raw test.
  // We handle keys at the top level, with values that are either:
  //   key: value
  //   key:\n  - item1\n  - item2
  //   key:\n  subkey: subvalue
  llvh::SmallVector<llvh::StringRef, 32> yamlLines;
  yaml.split(yamlLines, '\n');

  std::string currentKey;
  std::string currentValue;

  auto finishKey = [&]() {
    if (currentKey.empty())
      return;
    llvh::StringRef key(currentKey);
    llvh::StringRef val(currentValue);

    if (key == "description" || key == "info" || key == "commentary") {
      record.description = val.trim().str();
    } else if (key == "flags") {
      record.flags = parseYamlList(val);
    } else if (key == "features") {
      record.features = parseYamlList(val);
    } else if (key == "includes") {
      record.includes = parseYamlList(val);
    } else if (key == "negative") {
      // Parse sub-keys.
      llvh::SmallVector<llvh::StringRef, 4> negLines;
      val.split(negLines, '\n');
      for (auto line : negLines) {
        auto t = line.trim();
        if (t.startswith("phase:")) {
          record.negative.phase = t.drop_front(6).trim().str();
        } else if (t.startswith("type:")) {
          record.negative.errorType = t.drop_front(5).trim().str();
        }
      }
    }
    // Ignore other keys (es5id, esid, es6id, etc.)
    currentKey.clear();
    currentValue.clear();
  };

  for (auto line : yamlLines) {
    // Check if this is a top-level key (no leading whitespace, has colon).
    if (!line.empty() && line[0] != ' ' && line[0] != '\t') {
      finishKey();
      auto colonPos = line.find(':');
      if (colonPos != llvh::StringRef::npos) {
        currentKey = line.substr(0, colonPos).trim().str();
        auto rest = line.substr(colonPos + 1).trim();
        // Handle multi-line values (> or |)
        if (rest == ">" || rest == "|") {
          currentValue = "";
        } else {
          currentValue = rest.str();
        }
      }
    } else {
      // Continuation line for current key.
      if (!currentKey.empty()) {
        if (!currentValue.empty())
          currentValue += "\n";
        currentValue += line.str();
      }
    }
  }
  finishKey();

  // For raw tests, preserve the source exactly as-is (minus frontmatter).
  // For non-raw tests, strip the license header and leading blank lines.
  if (!record.isRaw()) {
    src = stripLicenseHeader(src);

    // Trim leading blank lines from result.
    while (!src.empty() && src[0] == '\n')
      src = src.substr(1);
  }

  record.src = src;

  return record;
}

} // namespace testrunner
} // namespace hermes

#endif // HERMES_TOOLS_TESTRUNNER_FRONTMATTER_H
