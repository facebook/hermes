/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_TESTRUNNER_SKIPLIST_H
#define HERMES_TOOLS_TESTRUNNER_SKIPLIST_H

#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/SourceErrorManager.h"

#include "llvh/ADT/StringRef.h"
#include "llvh/Support/MemoryBuffer.h"
#include "llvh/Support/raw_ostream.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace hermes {
namespace testrunner {

/// Why a test was skipped.
enum class SkipReason {
  NotSkipped,
  ManualSkipList,
  SkipList,
  LazySkipList,
  PermanentSkipList,
  HandlesanSkipList,
  UnsupportedFeature,
  PermanentUnsupportedFeature,
  IntlTests,
  PlatformSkipList,
};

/// Display name for a skip reason.
inline const char *skipReasonName(SkipReason r) {
  switch (r) {
    case SkipReason::NotSkipped:
      return "not_skipped";
    case SkipReason::ManualSkipList:
      return "manual_skip_list";
    case SkipReason::SkipList:
      return "skip_list";
    case SkipReason::LazySkipList:
      return "lazy_skip_list";
    case SkipReason::PermanentSkipList:
      return "permanent_skip_list";
    case SkipReason::HandlesanSkipList:
      return "handlesan_skip_list";
    case SkipReason::UnsupportedFeature:
      return "unsupported_features";
    case SkipReason::PermanentUnsupportedFeature:
      return "permanent_unsupported_features";
    case SkipReason::IntlTests:
      return "intl_tests";
    case SkipReason::PlatformSkipList:
      return "platform_skip_list";
  }
  return "unknown";
}

/// A category of skip paths with its associated reason.
struct SkipCategory {
  SkipReason reason;
  std::vector<std::string> paths;
};

/// Pre-built lookup structure for fast skip checks.
///
/// At load time, all skiplist JSON entries are flattened into per-category
/// path lists. The shouldSkip() method does substring matching (skiplist
/// path is a substring of the test path), matching the Python/Rust
/// implementations.
class Skiplist {
  /// Path-based skip categories.
  std::vector<SkipCategory> categories_;
  /// Paths where handle sanitizer should be disabled (not skipped).
  std::vector<std::string> handlesanPaths_;
  /// Features that cause a skip when present in test metadata.
  std::unordered_set<std::string> unsupportedFeatures_;
  /// Features that cause a permanent skip.
  std::unordered_set<std::string> permanentUnsupportedFeatures_;
  /// Features that the built Hermes binary supports, even if they appear in
  /// unsupported_features. Matching the Python runner, which queries
  /// `hermes --version` to discover supported features and excludes them
  /// from feature-based skipping.
  std::unordered_set<std::string> supportedFeatures_;

  /// Extract all string values from a JSON array of skip entries into a
  /// container. Each entry can be either a bare string or an object with a
  /// "paths" array. The Inserter is called with each extracted std::string.
  template <typename Inserter>
  static void flattenStrings(parser::JSONArray *arr, Inserter inserter) {
    if (!arr)
      return;
    for (size_t i = 0; i < arr->size(); ++i) {
      auto *val = arr->at(i);
      if (auto *str = llvh::dyn_cast<parser::JSONString>(val)) {
        inserter(str->str().str());
      } else if (auto *obj = llvh::dyn_cast<parser::JSONObject>(val)) {
        auto *pathsVal = obj->get("paths");
        if (auto *pathsArr =
                llvh::dyn_cast_or_null<parser::JSONArray>(pathsVal)) {
          for (size_t j = 0; j < pathsArr->size(); ++j) {
            if (auto *s = llvh::dyn_cast<parser::JSONString>(pathsArr->at(j))) {
              inserter(s->str().str());
            }
          }
        }
      }
    }
  }

  /// Extract all path strings from a JSON array into a vector.
  static void flattenEntries(
      parser::JSONArray *arr,
      std::vector<std::string> &out) {
    flattenStrings(arr, [&out](std::string s) { out.push_back(std::move(s)); });
  }

  /// Extract feature names from a JSON array into a set.
  static void flattenFeatures(
      parser::JSONArray *arr,
      std::unordered_set<std::string> &out) {
    flattenStrings(arr, [&out](std::string s) { out.insert(std::move(s)); });
  }

  /// Get a JSON array from the root object by key, or nullptr.
  static parser::JSONArray *getArray(
      parser::JSONObject *root,
      const char *key) {
    auto *val = root->get(key);
    return llvh::dyn_cast_or_null<parser::JSONArray>(val);
  }

  /// Load named path-based skip categories from the root JSON object.
  void loadPathCategories(parser::JSONObject *root) {
    struct {
      const char *key;
      SkipReason reason;
    } pathCategories[] = {
        {"manual_skip_list", SkipReason::ManualSkipList},
        {"skip_list", SkipReason::SkipList},
        {"lazy_skip_list", SkipReason::LazySkipList},
        {"permanent_skip_list", SkipReason::PermanentSkipList},
        {"intl_tests", SkipReason::IntlTests},
        // handlesan_skip_list is loaded separately below — those tests
        // run with handle sanitizer disabled, not skipped.
    };

    for (const auto &cat : pathCategories) {
      SkipCategory sc;
      sc.reason = cat.reason;
      flattenEntries(getArray(root, cat.key), sc.paths);
      if (!sc.paths.empty())
        categories_.push_back(std::move(sc));
    }

    // Load handlesan paths separately — these tests are run (not skipped)
    // but with GC handle sanitization disabled.
    flattenEntries(getArray(root, "handlesan_skip_list"), handlesanPaths_);
  }

  /// Load platform-specific skip paths from the root JSON object.
  void loadPlatformSkipList(parser::JSONObject *root) {
#if defined(__linux__)
    const char *platform = "linux";
#elif defined(__APPLE__)
    const char *platform = "darwin";
#elif defined(_WIN32)
    const char *platform = "win32";
#else
    const char *platform = nullptr;
#endif
    if (!platform)
      return;

    auto *platformObj = llvh::dyn_cast_or_null<parser::JSONObject>(
        root->get("platform_skip_list"));
    if (!platformObj)
      return;

    auto *platformArr =
        llvh::dyn_cast_or_null<parser::JSONArray>(platformObj->get(platform));
    if (!platformArr)
      return;

    SkipCategory sc;
    sc.reason = SkipReason::PlatformSkipList;
    flattenEntries(platformArr, sc.paths);
    if (!sc.paths.empty())
      categories_.push_back(std::move(sc));
  }

 public:
  Skiplist() = default;

  /// Load and parse a skiplist from a JSON file.
  /// Returns true on success, false on parse error.
  bool load(llvh::StringRef path) {
    auto fileBuf = llvh::MemoryBuffer::getFile(path);
    if (!fileBuf) {
      llvh::errs() << "Error: cannot read skiplist file '" << path
                   << "': " << fileBuf.getError().message() << "\n";
      return false;
    }

    parser::JSLexer::Allocator alloc;
    parser::JSONFactory factory(alloc);
    SourceErrorManager sm;
    parser::JSONParser jsonParser(factory, *fileBuf.get(), sm);
    auto parsed = jsonParser.parse();
    if (!parsed) {
      llvh::errs() << "Error: failed to parse skiplist JSON\n";
      return false;
    }

    auto *root = llvh::dyn_cast<parser::JSONObject>(parsed.getValue());
    if (!root) {
      llvh::errs() << "Error: skiplist JSON root is not an object\n";
      return false;
    }

    loadPathCategories(root);
    loadPlatformSkipList(root);

    flattenFeatures(
        getArray(root, "unsupported_features"), unsupportedFeatures_);
    flattenFeatures(
        getArray(root, "permanent_unsupported_features"),
        permanentUnsupportedFeatures_);

    // Populate supported features based on compile-time configuration.
    // This mirrors the Python runner's get_hermes_supported_test262_features(),
    // which queries `hermes --version` and maps Hermes feature names to
    // test262 feature names. Since the C++ runner runs in-process, we use
    // compile-time preprocessor flags instead.
#ifdef HERMES_ENABLE_UNICODE_REGEXP_PROPERTY_ESCAPES
    supportedFeatures_.insert("regexp-unicode-property-escapes");
#endif

    return true;
  }

  /// Check if a test path should be skipped based on path matching.
  /// Uses substring matching: a skiplist entry matches if it is contained
  /// within the test path (matching Python's `value in test_or_feature`).
  SkipReason shouldSkipPath(llvh::StringRef testPath) const {
    for (const auto &cat : categories_) {
      for (const auto &skipPath : cat.paths) {
        if (testPath.contains(skipPath)) {
          return cat.reason;
        }
      }
    }
    return SkipReason::NotSkipped;
  }

  /// Check if a test path should be skipped by any non-intl category.
  /// This mirrors the Python runner's two-pass design: first check
  /// skip_list/permanent_skip_list/manual_skip_list/platform_skip_list,
  /// then check intl_tests separately. Used when --test-intl bypasses
  /// IntlTests but platform-specific skips must still be honored.
  SkipReason shouldSkipPathNonIntl(llvh::StringRef testPath) const {
    for (const auto &cat : categories_) {
      if (cat.reason == SkipReason::IntlTests)
        continue;
      for (const auto &skipPath : cat.paths) {
        if (testPath.contains(skipPath)) {
          return cat.reason;
        }
      }
    }
    return SkipReason::NotSkipped;
  }

  /// Check if a test should be skipped because it uses an unsupported feature.
  /// Features that the built binary actually supports (based on compile-time
  /// config) are not skipped, matching the Python runner's behavior.
  SkipReason shouldSkipFeature(llvh::StringRef feature) const {
    std::string feat = feature.str();
    // If the binary supports this feature, don't skip regardless of skiplist.
    if (supportedFeatures_.count(feat))
      return SkipReason::NotSkipped;
    if (permanentUnsupportedFeatures_.count(feat))
      return SkipReason::PermanentUnsupportedFeature;
    if (unsupportedFeatures_.count(feat))
      return SkipReason::UnsupportedFeature;
    return SkipReason::NotSkipped;
  }

  /// Check if a test should disable GC handle sanitization.
  /// Tests in handlesan_skip_list should run but with sanitize rate = 0.
  bool shouldDisableHandleSan(llvh::StringRef testPath) const {
    for (const auto &p : handlesanPaths_) {
      if (testPath.contains(p))
        return true;
    }
    return false;
  }

  /// Get counts for reporting.
  size_t totalSkipPaths() const {
    size_t total = 0;
    for (const auto &cat : categories_)
      total += cat.paths.size();
    return total;
  }

  size_t totalUnsupportedFeatures() const {
    return unsupportedFeatures_.size() + permanentUnsupportedFeatures_.size();
  }
};

} // namespace testrunner
} // namespace hermes

#endif // HERMES_TOOLS_TESTRUNNER_SKIPLIST_H
