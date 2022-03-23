/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/IdentifierTable.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringRefUtils.h"
#include "hermes/VM/StringView.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace hermes;
using namespace hermes::vm;

namespace {

using IdentifierTableLargeHeapTest = LargeHeapRuntimeTestFixture;

TEST_F(IdentifierTableLargeHeapTest, LookupTest) {
  IdentifierTable &table = runtime.getIdentifierTable();

  uint32_t predefinedCount = table.getSymbolsEnd();

  UTF16Ref a{u"foo", 3};
  UTF16Ref b{u"ab", 2};
  UTF16Ref c{u"foo", 3};

  SymbolID sa = table.getSymbolHandle(runtime, a).getValue().get();
  ASSERT_GE(sa.unsafeGetIndex(), predefinedCount);
  SymbolID sb = table.getSymbolHandle(runtime, b).getValue().get();
  ASSERT_GE(sb.unsafeGetIndex(), predefinedCount);

  EXPECT_EQ(0u, sa.unsafeGetIndex() - predefinedCount);
  EXPECT_EQ(1u, sb.unsafeGetIndex() - predefinedCount);
  EXPECT_EQ(
      0u,
      table.getSymbolHandle(runtime, a).getValue().get().unsafeGetIndex() -
          predefinedCount);
  EXPECT_EQ(
      0u,
      table.getSymbolHandle(runtime, c).getValue().get().unsafeGetIndex() -
          predefinedCount);

  auto d = StringPrimitive::createNoThrow(runtime, llvh::StringRef("foo"));
  EXPECT_EQ(
      0u,
      table.getSymbolHandleFromPrimitive(runtime, d)
              .getValue()
              .get()
              .unsafeGetIndex() -
          predefinedCount);

  auto e = StringPrimitive::createNoThrow(runtime, llvh::StringRef("ab"));
  EXPECT_EQ(
      1u,
      table.getSymbolHandleFromPrimitive(runtime, e)
              .getValue()
              .get()
              .unsafeGetIndex() -
          predefinedCount);

  EXPECT_TRUE(table.getStringView(runtime, sa).equals(a));
  EXPECT_TRUE(table.getStringView(runtime, sb).equals(b));
  SmallU16String<8> tmp;
  table.getStringView(runtime, sa).appendUTF16String(tmp);
  EXPECT_EQ(a, tmp.arrayRef());
  tmp.clear();
  table.getStringView(runtime, sb).appendUTF16String(tmp);
  EXPECT_EQ(b, tmp.arrayRef());

  // Ensure allocations are aligned.
  EXPECT_EQ(
      0u,
      (uint64_t)runtime.getStringPrimFromSymbolID(sa) % (uint64_t)HeapAlign);
  EXPECT_EQ(
      0u,
      (uint64_t)runtime.getStringPrimFromSymbolID(sb) % (uint64_t)HeapAlign);
}

using IdentifierTableTest = RuntimeTestFixture;

TEST_F(IdentifierTableTest, NotUniquedSymbol) {
  auto &idTable = runtime.getIdentifierTable();

  {
    ASCIIRef asdf{"asdf", 4};
    Handle<StringPrimitive> id1 = runtime.makeHandle<StringPrimitive>(
        *StringPrimitive::create(runtime, asdf));
    Handle<SymbolID> sym =
        runtime.makeHandle(*idTable.createNotUniquedSymbol(runtime, id1));
    EXPECT_TRUE((*sym).isNotUniqued());
    EXPECT_FALSE((*sym).isUniqued());
    EXPECT_TRUE(idTable.getStringView(runtime, *sym).equals(asdf));
  }
}

TEST(IdentifierTableDeathTest, LazyExternalSymbolTooBig) {
  auto fn = [] {
    auto rt = Runtime::create(
        RuntimeConfig::Builder().withGCConfig(kTestGCConfig).build());
    auto &runtime = *rt;
    GCScope gcScope{runtime};
    auto &idTable = runtime.getIdentifierTable();

    const auto extSize = (1 << 24) +
        std::max(kTestGCConfig.getMaxHeapSize(),
                 toRValue(StringPrimitive::EXTERNAL_STRING_THRESHOLD));

    // A string of this size is definitely too big to be allocated.
    ASSERT_FALSE(runtime.getHeap().canAllocExternalMemory(extSize));

    std::string buf(extSize, '\0');
    ASCIIRef ref{buf.data(), extSize};

    SymbolID symbol = idTable.registerLazyIdentifier(ref);
    idTable.getStringPrim(runtime, symbol);
  };
  EXPECT_DEATH_IF_SUPPORTED(fn(), "Unhandled out of memory exception");
}

// Verifies that SymbolIDs are allocated consecutively, increasing from zero, as
// long as none have been freed.
TEST_F(IdentifierTableTest, ConsecutiveIncreasingSymbolIDAlloc) {
  IdentifierTable idTable;

  // Backing store for StringRefs
  std::vector<std::string> ascii;
  std::vector<std::u16string> utf16;

  for (size_t i = 0; i < 100; ++i) {
    std::stringstream ssa;
    ssa << "ascii-" << i;
    ascii.emplace_back(ssa.str());

    std::stringstream ssu;
    ssu << "utf16-" << i;
    auto abuf = ssu.str();
    std::u16string buf;
    convertUTF8WithSurrogatesToUTF16(
        std::back_inserter(buf), abuf.data(), abuf.data() + abuf.size());

    utf16.emplace_back(std::move(buf));
  }

  { // Add refs to the ID Table.  First time round, allocate all new IDs.
    size_t idx = 0;
    for (auto &s : ascii) {
      auto r = createASCIIRef(s.c_str());
      EXPECT_EQ(idTable.registerLazyIdentifier(r).unsafeGetIndex(), idx++)
          << "Uniqued ASCII First Round";
    }

    for (auto &s : utf16) {
      auto r = createUTF16Ref(s.c_str());
      EXPECT_EQ(idTable.registerLazyIdentifier(r).unsafeGetIndex(), idx++)
          << "Uniqued UTF16 First Round";
    }

    for (auto &s : ascii) {
      auto r = createASCIIRef(s.c_str());
      EXPECT_EQ(idTable.createNotUniquedLazySymbol(r).unsafeGetIndex(), idx++)
          << "Not Uniqued ASCII First Round";
    }
  }

  { // Next time around: The IDs should be the same as before for the uniqued
    // SymbolIDs.
    size_t idx = 0;
    for (auto &s : ascii) {
      auto r = createASCIIRef(s.c_str());
      EXPECT_EQ(idTable.registerLazyIdentifier(r).unsafeGetIndex(), idx++)
          << "Uniqued ASCII Second Round";
    }

    for (auto &s : utf16) {
      auto r = createUTF16Ref(s.c_str());
      EXPECT_EQ(idTable.registerLazyIdentifier(r).unsafeGetIndex(), idx++)
          << "Uniqued UTF16 Second Round";
    }

    // These symbols are not uniqued so they are going to get re-allocated every
    // time, although still linearly.
    idx = idTable.getSymbolsEnd();
    for (auto &s : ascii) {
      auto r = createASCIIRef(s.c_str());
      EXPECT_EQ(idTable.createNotUniquedLazySymbol(r).unsafeGetIndex(), idx++)
          << "Not Uniqued ASCII Second Round";
    }
  }
}

} // namespace
