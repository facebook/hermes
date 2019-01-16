#include "hermes/Support/HashString.h"

#include <limits>

#include "gtest/gtest.h"

using namespace hermes;

namespace {

static llvm::ArrayRef<char> makeArrayRef(const char *s) {
  return {s, strlen(s)};
}

TEST(HashStringTest, Constexpr) {
  EXPECT_EQ(hashString(makeArrayRef("abc")), constexprHashString("abc"));
  EXPECT_EQ(hashString(makeArrayRef("")), constexprHashString(""));
  EXPECT_EQ(
      hashString(makeArrayRef("1234567")), constexprHashString("1234567"));
}

} // end anonymous namespace
