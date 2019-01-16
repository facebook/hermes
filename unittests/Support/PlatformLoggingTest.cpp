#include "hermes/Platform/Logging.h"

#include "gtest/gtest.h"

namespace hermes {
namespace {

TEST(PlatformLoggingTest, IsExpression) {
  // Ensure hermesLog is an expression by ensuring this compiles.
  (hermesLog("unittest", "Hello %s", "World"));
}

} // end anonymous namespace
} // namespace hermes
