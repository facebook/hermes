/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// Parts of regex, at least as used by gmock, appear to be unsupported on
/// Windows. So we'll exempt this test on Windows.
#ifndef _WINDOWS

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AlignedHeapSegment.h"
#include "hermes/VM/GC.h"
#include "hermes/VM/LimitedStorageProvider.h"
#include "hermes/VM/PointerBase.h"

#include <deque>

using namespace hermes;
using namespace hermes::vm;

using ::testing::MatchesRegex;

namespace {

// We make this not FixedSize, to allow direct allocation in the old generation.
using SegmentCell = EmptyCell<AlignedHeapSegment::maxSize()>;

class TestCrashManager : public CrashManager {
 public:
  void registerMemory(void *, size_t) override {}
  void unregisterMemory(void *) override {}
  CallbackKey registerCallback(CallbackFunc /*callback*/) override {
    return 0;
  }
  void unregisterCallback(CallbackKey /*key*/) override {}
  void setHeapInfo(const HeapInformation & /*heapInfo*/) override {}

  void setCustomData(const char *key, const char *value) override {
    customData_[std::string(key)] = std::string(value);
  }
  void setContextualCustomData(const char *key, const char *value) override {
    contextualCustomData_[std::string(key)] = std::string(value);
  }
  void removeCustomData(const char *key) override {
    customData_.erase(std::string(key));
  }
  void removeContextualCustomData(const char *key) override {
    contextualCustomData_.erase(std::string(key));
  }

  const std::unordered_map<std::string, std::string> &customData() {
    return customData_;
  }

  const std::unordered_map<std::string, std::string> &contextualCustomData() {
    return contextualCustomData_;
  }

 private:
  std::unordered_map<std::string, std::string> customData_;
  std::unordered_map<std::string, std::string> contextualCustomData_;
};

#ifndef HERMESVM_SANITIZE_HANDLES

static ::testing::AssertionResult validJSON(const std::string &json) {
  SourceErrorManager sm;
  hermes::parser::JSONFactory::Allocator alloc;
  hermes::parser::JSONFactory factory{alloc};
  hermes::parser::JSONParser parser{factory, json, sm};
  if (!parser.parse()) {
    return ::testing::AssertionFailure() << "Invalid JSON: \"" << json << "\"";
  }

  return ::testing::AssertionSuccess();
}

/// We are able to materialize every segment.
TEST(CrashManagerTest, HeapExtentsCorrect) {
  /// We need a max heap that's bigger than normal
  GCConfig gcConfig = GCConfig::Builder(kTestGCConfigBuilder)
                          .withName("XYZ")
                          .withInitHeapSize(kInitHeapLarge)
                          .withMaxHeapSize(1 << 28)
                          .build();
  auto testCrashMgr = std::make_shared<TestCrashManager>();
  auto runtime = DummyRuntime::create(
      gcConfig, DummyRuntime::defaultProvider(), testCrashMgr);
  DummyRuntime &rt = *runtime;

  GCScope scope{&rt};

  // Allocate 25 segments.  By the time we're done, this should fill
  // the YG (which will have grown to a full segment size), and 24 OG
  // segments.
  for (size_t i = 0; i < 8; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }
  auto marker = scope.createMarker();
  (void)marker;
  for (size_t i = 0; i < 17; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }
  // This function isn't used in all paths.
  (void)validJSON;

#ifdef HERMESVM_GC_HADES
  const auto &contextualCustomData = testCrashMgr->contextualCustomData();
  EXPECT_EQ(26, contextualCustomData.size());
  const std::string expectedKeyBase = "XYZ:HeapSegment:";
  for (int i = 0; i < 26; i++) {
    const std::string expectedKey =
        expectedKeyBase + (i == 0 ? std::string("YG") : std::to_string(i));
    std::string actual = contextualCustomData.at(expectedKey);
    void *ptr = nullptr;
    int numArgsWritten = std::sscanf(actual.c_str(), "%p", &ptr);
    EXPECT_EQ(numArgsWritten, 1);
    // The pointer value itself doesn't matter, as long as it is one of the
    // segments in the heap. That would require exposing more GC APIs though,
    // so just check that it was parsed as non-null.
    EXPECT_NE(ptr, nullptr);
  }
#endif
}
#endif

#ifdef HERMESVM_GC_HADES
TEST(CrashManagerTest, PromotedYGHasCorrectName) {
  // Turn on the "direct to OG" allocation feature.
  GCConfig gcConfig = GCConfig::Builder(kTestGCConfigBuilder)
                          .withName("XYZ")
                          .withInitHeapSize(1 << 26)
                          .withMaxHeapSize(1 << 28)
                          .withAllocInYoung(false)
                          .withRevertToYGAtTTI(true)
                          .build();
  auto testCrashMgr = std::make_shared<TestCrashManager>();
  auto runtime = DummyRuntime::create(
      gcConfig, DummyRuntime::defaultProvider(), testCrashMgr);
  DummyRuntime &rt = *runtime;

  GCScope scope{&rt};

  // Fill up YG at least once, to make sure promotion keeps the right name.
  for (size_t i = 0; i < 3; ++i) {
    rt.makeHandle(SegmentCell::create(rt));
  }

  const auto &contextualCustomData = testCrashMgr->contextualCustomData();
  EXPECT_EQ(4, contextualCustomData.size());
  // Make sure the value for YG is the actual YG segment.
  const std::string ygAddress = contextualCustomData.at("XYZ:HeapSegment:YG");
  void *ptr = nullptr;
  int numArgsWritten = std::sscanf(ygAddress.c_str(), "%p", &ptr);
  EXPECT_EQ(numArgsWritten, 1);
  EXPECT_TRUE(rt.getHeap().inYoungGen(ptr));

  // Test if all the segments are numbered correctly.
  EXPECT_EQ(contextualCustomData.count("XYZ:HeapSegment:1"), 1);
  EXPECT_EQ(contextualCustomData.count("XYZ:HeapSegment:2"), 1);
  EXPECT_EQ(contextualCustomData.count("XYZ:HeapSegment:3"), 1);
}
#endif

TEST(CrashManagerTest, GCNameIncluded) {
  GCConfig gcConfig =
      GCConfig::Builder(kTestGCConfigBuilder).withName("XYZ").build();
  auto testCrashMgr = std::make_shared<TestCrashManager>();
  auto runtime = DummyRuntime::create(
      gcConfig, DummyRuntime::defaultProvider(), testCrashMgr);

  const auto &crashData = testCrashMgr->customData();
  auto gcName = crashData.find("HermesGC");
  // Each GC can set its name as it likes, the only real requirement is that
  // it exists and is distinct. Since we can test more than one GC per build,
  // we'll just test the "exists" part.
  ASSERT_NE(gcName, crashData.end()) << "HermesGC key not found in crash data";
  EXPECT_NE(gcName->second, "") << "HermesGC value is empty";
}

} // namespace

#endif // _WINDOWS
