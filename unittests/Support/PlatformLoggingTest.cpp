/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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
