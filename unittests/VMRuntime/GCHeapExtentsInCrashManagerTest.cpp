/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(HERMESVM_GC_NONCONTIG_GENERATIONAL) || defined(HERMESVM_GC_HADES)

/// Parts of regex, at least as used by gmock, appear to be unsupported on
/// Windows. So we'll exempt this test on Windows.
#ifndef _WINDOWS

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "EmptyCell.h"
#include "TestHelpers.h"
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

const MetadataTableForTests getMetadataTable() {
  static const Metadata storage[] = {
      Metadata() // Uninitialized
  };
  return MetadataTableForTests(storage);
}

// We make this not FixedSize, to allow direct allocation in the old generation.
using SegmentCell = VarSizedEmptyCell<AlignedHeapSegment::maxSize()>;

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

/// We are able to materialize every segment.
TEST(GCHeapExtentsInCrashManagerTest, HeapExtentsCorrect) {
  /// We need a max heap that's bigger than normal
  GCConfig gcConfig = GCConfig::Builder(kTestGCConfigBuilder)
                          .withName("XYZ")
                          .withInitHeapSize(kInitHeapLarge)
                          .withMaxHeapSize(1 << 28)
                          .build();
  auto testCrashMgr = std::make_shared<TestCrashManager>();
  auto runtime = DummyRuntime::create(
      getMetadataTable(),
      gcConfig,
      DummyRuntime::defaultProvider(),
      testCrashMgr);
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  // Allocate 25 segments.  By the time we're done, this should fill
  // the YG (which will have grown to a full segment size), and 24 OG
  // segments.
  for (size_t i = 0; i < 25; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }
#ifdef HERMESVM_GC_NONCONTIG_GENERATIONAL
  EXPECT_EQ(4, testCrashMgr->customData().size());

  const std::string expectedYgKeyStr = "XYZ:HeapSegments_YG";
  const std::string oneExtent =
      "\\{lo: \"0x[a-f0-9]*\", hi: \"0x[a-f0-9]*\"\\}";

  EXPECT_NE(
      testCrashMgr->customData().end(),
      testCrashMgr->customData().find(expectedYgKeyStr));
  EXPECT_THAT(
      testCrashMgr->customData().at(expectedYgKeyStr),
      MatchesRegex(oneExtent + "$"));

  const std::string expectedOgKeyStr = "XYZ:HeapSegments_OG";
  const std::string expectedOgKeyStr0 = expectedOgKeyStr + ":0";
  const std::string expectedOgKeyStr10 = expectedOgKeyStr + ":10";
  const std::string expectedOgKeyStr20 = expectedOgKeyStr + ":20";

  const std::string leftBracket = "\\[";
  const std::string rightBracket = "\\]";

  const std::string fourExtentsBody =
      oneExtent + "," + oneExtent + "," + oneExtent + "," + oneExtent;
  const std::string fourExtents = leftBracket + fourExtentsBody + rightBracket;

  const std::string fiveExtentsBody = fourExtentsBody + "," + oneExtent;
  const std::string fiveExtents = leftBracket + fiveExtentsBody + rightBracket;

  const std::string eightExtentsBody = fourExtentsBody + "," + fourExtentsBody;
  const std::string eightExtents =
      leftBracket + eightExtentsBody + rightBracket;

  const std::string tenExtentsBody = fiveExtentsBody + "," + fiveExtentsBody;
  const std::string tenExtents = leftBracket + tenExtentsBody + rightBracket;

  EXPECT_NE(
      testCrashMgr->customData().end(),
      testCrashMgr->customData().find(expectedOgKeyStr0));
  EXPECT_THAT(
      testCrashMgr->customData().at(expectedOgKeyStr0),
      MatchesRegex(tenExtents + "$"));
  EXPECT_NE(
      testCrashMgr->customData().end(),
      testCrashMgr->customData().find(expectedOgKeyStr10));
  EXPECT_THAT(
      testCrashMgr->customData().at(expectedOgKeyStr10),
      MatchesRegex(tenExtents + "$"));
  EXPECT_NE(
      testCrashMgr->customData().end(),
      testCrashMgr->customData().find(expectedOgKeyStr20));
  // "four" because one of the 25 segments is the young gen.
  EXPECT_THAT(
      testCrashMgr->customData().at(expectedOgKeyStr20),
      MatchesRegex(fourExtents + "$"));

  // Now allocate one more, make sure we now have 5 in the 20 key.
  roots.push_back(SegmentCell::create(rt));
  rt.pointerRoots.push_back(&roots.back());

  EXPECT_THAT(
      testCrashMgr->customData().at(expectedOgKeyStr0),
      MatchesRegex(tenExtents + "$"));
  EXPECT_THAT(
      testCrashMgr->customData().at(expectedOgKeyStr10),
      MatchesRegex(tenExtents + "$"));
  EXPECT_THAT(
      testCrashMgr->customData().at(expectedOgKeyStr20),
      MatchesRegex(fiveExtents + "$"));

  /// Now erase enough roots so that we only have 7 OG segments remaining
  /// (to pick a fairly random number).  Then do a GC.  The old gen
  /// will have those 7 filled segments, plus an empty allocation
  /// segment.  Make sure only those are registered with the crash manager.
  for (size_t i = 0; i < (25 - 7); i++) {
    roots.pop_back();
    rt.pointerRoots.pop_back();
  }
  rt.collect();

  EXPECT_EQ(2, testCrashMgr->customData().size());

  EXPECT_NE(
      testCrashMgr->customData().end(),
      testCrashMgr->customData().find(expectedYgKeyStr));
  EXPECT_THAT(
      testCrashMgr->customData().at(expectedYgKeyStr),
      MatchesRegex(oneExtent + "$"));

  EXPECT_NE(
      testCrashMgr->customData().end(),
      testCrashMgr->customData().find(expectedOgKeyStr0));
  EXPECT_THAT(
      testCrashMgr->customData().at(expectedOgKeyStr0),
      MatchesRegex(eightExtents + "$"));
#elif defined(HERMESVM_GC_HADES)
  const auto &contextualCustomData = testCrashMgr->contextualCustomData();
  EXPECT_EQ(25, contextualCustomData.size());
  const std::string expectedKeyBase = "XYZ:HeapSegment:";
  for (int i = 0; i < 25; i++) {
    const std::string expectedKey =
        expectedKeyBase + (i == 0 ? std::string("YG") : oscompat::to_string(i));
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

#ifdef HERMESVM_GC_HADES
TEST(GCHeapExtentsInCrashManagerTest, PromotedYGHasCorrectName) {
  // Turn on the "direct to OG" allocation feature.
  GCConfig gcConfig = GCConfig::Builder(kTestGCConfigBuilder)
                          .withName("XYZ")
                          .withInitHeapSize(kInitHeapLarge)
                          .withMaxHeapSize(1 << 28)
                          .withAllocInYoung(false)
                          .withRevertToYGAtTTI(true)
                          .build();
  auto testCrashMgr = std::make_shared<TestCrashManager>();
  auto runtime = DummyRuntime::create(
      getMetadataTable(),
      gcConfig,
      DummyRuntime::defaultProvider(),
      testCrashMgr);
  DummyRuntime &rt = *runtime;

  std::deque<GCCell *> roots;

  // Fill up YG at least once, to make sure promotion keeps the right name.
  for (size_t i = 0; i < 3; ++i) {
    roots.push_back(SegmentCell::create(rt));
    rt.pointerRoots.push_back(&roots.back());
  }

  const auto &contextualCustomData = testCrashMgr->contextualCustomData();
  EXPECT_EQ(3, contextualCustomData.size());
  // Make sure the value for YG is the actual YG segment.
  const std::string ygAddress = contextualCustomData.at("XYZ:HeapSegment:YG");
  void *ptr = nullptr;
  int numArgsWritten = std::sscanf(ygAddress.c_str(), "%p", &ptr);
  EXPECT_EQ(numArgsWritten, 1);
  EXPECT_TRUE(rt.getHeap().inYoungGen(ptr));

  // Test if the other segments are numbered correctly.
  EXPECT_EQ(contextualCustomData.count("XYZ:HeapSegment:1"), 1);
  EXPECT_EQ(contextualCustomData.count("XYZ:HeapSegment:2"), 1);
}
#endif

} // namespace

#endif // _WINDOWS

#endif // HERMESVM_GC_NONCONTIG_GENERATIONAL || HERMESVM_GC_HADES
