#include "hermes/VM/SymbolID.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

TEST(SymbolIDTest, ExternalTest) {
  auto id1 = SymbolID::unsafeCreateExternal(0x50000001);
  EXPECT_TRUE(id1.isExternal());
  EXPECT_FALSE(id1.isInternal());
  EXPECT_EQ(0xd0000001, id1.unsafeGetRaw());
  EXPECT_EQ(0x50000001, id1.unsafeGetIndex());
  auto id2 = SymbolID::unsafeCreate(0x40000007);
  EXPECT_TRUE(id2.isInternal());
  EXPECT_FALSE(id2.isExternal());
  EXPECT_EQ(0x40000007, id2.unsafeGetRaw());
  EXPECT_EQ(0x40000007, id2.unsafeGetIndex());
}

} // namespace
