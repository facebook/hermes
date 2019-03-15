/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/IdentifierTable.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using IdentifierTableLargeHeapTest = LargeHeapRuntimeTestFixture;

TEST_F(IdentifierTableLargeHeapTest, LookupTest) {
  IdentifierTable &table = runtime->getIdentifierTable();

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

  auto d = StringPrimitive::createNoThrow(runtime, llvm::StringRef("foo"));
  EXPECT_EQ(
      0u,
      table.getSymbolHandleFromPrimitive(runtime, d)
              .getValue()
              .get()
              .unsafeGetIndex() -
          predefinedCount);

  auto e = StringPrimitive::createNoThrow(runtime, llvm::StringRef("ab"));
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
  table.getStringView(runtime, sa).copyUTF16String(tmp);
  EXPECT_EQ(a, tmp.arrayRef());
  tmp.clear();
  table.getStringView(runtime, sb).copyUTF16String(tmp);
  EXPECT_EQ(b, tmp.arrayRef());

  // Ensure allocations are aligned.
  EXPECT_EQ(
      0u,
      (uint64_t)runtime->getStringPrimFromSymbolID(sa) % (uint64_t)HeapAlign);
  EXPECT_EQ(
      0u,
      (uint64_t)runtime->getStringPrimFromSymbolID(sb) % (uint64_t)HeapAlign);
}

using IdentifierTableNotUniquedTest = RuntimeTestFixture;

TEST_F(IdentifierTableNotUniquedTest, NotUniquedSymbol) {
  auto &idTable = runtime->getIdentifierTable();

  {
    ASCIIRef asdf{"asdf", 4};
    Handle<StringPrimitive> id1 = runtime->makeHandle<StringPrimitive>(
        *StringPrimitive::create(runtime, asdf));
    Handle<SymbolID> sym =
        runtime->makeHandle(*idTable.createNotUniquedSymbol(runtime, id1));
    EXPECT_TRUE((*sym).isNotUniqued());
    EXPECT_FALSE((*sym).isUniqued());
    EXPECT_TRUE(idTable.getStringView(runtime, *sym).equals(asdf));
  }
}

TEST(IdentifierTableTest, LazyExternalSymbolTooBig) {
  auto &rtConfig = kTestRTConfig;
  auto rt = Runtime::create(rtConfig);
  Runtime *runtime = rt.get();
  GCScope gcScope{runtime};
  auto &idTable = runtime->getIdentifierTable();

  static const auto kExtStringThreshold =
      StringPrimitive::EXTERNAL_STRING_THRESHOLD;
  const auto extSize = 1 +
      std::max(rtConfig.getGCConfig().getMaxHeapSize(), kExtStringThreshold);

  // A string of this size is definitely too big to be allocated.
  ASSERT_TRUE(StringPrimitive::isExternalLength(extSize));
  ASSERT_FALSE(runtime->getHeap().canAllocExternalMemory(extSize));

  auto buf = reinterpret_cast<char *>(malloc(extSize));
  ASSERT_NE(nullptr, buf);
  ASCIIRef ref{buf, extSize};

  SymbolID symbol = idTable.registerLazyIdentifier(ref);

  EXPECT_DEATH(
      { idTable.getStringPrim(runtime, symbol); },
      "Unhandled out of memory exception");
}

} // namespace
