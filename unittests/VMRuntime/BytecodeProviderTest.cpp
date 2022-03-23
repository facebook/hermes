/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#include "TestHelpers1.h"

using namespace hermes;
using namespace hermes::vm;
using namespace hermes::hbc;

namespace {

using BytecodeProviderTest = LargeHeapRuntimeTestFixture;

TEST_F(BytecodeProviderTest, IdentifierHashesPreserved) {
  // Test that running a bytecode file twice doesn't modify identifier hashes.
  uint32_t identifierCount = 4096;

  // Construct JS source containing lots of identifiers.
  std::stringstream source;
  source << "var obj = obj || {}; var sum = 0;\n";
  for (uint32_t i = 0; i < identifierCount; i++) {
    std::string objident = "obj.ident" + std::to_string(i);

    // Increment each property:  obj.ident3 = (obj.ident3 || 0) + 1;
    source << objident << " = (" << objident << " || 0) + 1;\n";

    // Sum them.
    source << "sum += " << objident << ";\n";
  }
  // 'return' the sum.
  source << "sum\n";

  // Compile it to bytecode.
  const std::vector<uint8_t> bytecode = bytecodeForSource(source.str().c_str());
  std::shared_ptr<BCProviderFromBuffer> bcProvider =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
          std::make_unique<Buffer>(&bytecode[0], bytecode.size()))
          .first;
  ASSERT_TRUE(nullptr != bcProvider);

  // Run it multiple times.
  // Each time the identifiers are incremented, so we expect the sum to be the
  // number of identifiers times the number of iterations.
  for (uint32_t i = 1; i <= 64; i++) {
    auto cr = runtime.runBytecode(
        bcProvider,
        RuntimeModuleFlags{},
        "sourceURL",
        runtime.makeNullHandle<Environment>());
    ASSERT_TRUE(cr == ExecutionStatus::RETURNED);
    EXPECT_EQ(i * identifierCount, cr->getNumberAs<uint32_t>());
  }
}

} // namespace
