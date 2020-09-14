/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/DiscoverBB.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"

#include "../TestHelpers.h"
#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

TEST(DiscoverBBTest, SmokeTest) {
  auto rt = Runtime::create(kTestRTConfigLargeHeap);
  Runtime &runtime = *rt;

  hermes::hbc::CompileFlags runFlags;
  runFlags.optimize = true;
  (void)runtime.run(
      "function bench (lc, fc) {\n"
      "    var n, fact;\n"
      "    var res = 0;\n"
      "    while (--lc >= 0) {\n"
      "        n = fc;\n"
      "        fact = n;\n"
      "        while (--n > 1)\n"
      "            fact *= n;\n"
      "        res += fact;\n"
      "    }\n"
      "    return res;\n"
      "};\n"
      "bench(1, 1);",
      "",
      runFlags);

  GCScope gcScope{&runtime};
  auto benchSym = runtime.getIdentifierTable().getSymbolHandle(
      &runtime, createASCIIRef("bench"));
  ASSERT_EQ(ExecutionStatus::RETURNED, benchSym);

  auto propRes = JSObject::getNamed_RJS(
      runtime.getGlobal(), &runtime, *benchSym.getValue());
  ASSERT_EQ(ExecutionStatus::RETURNED, propRes.getStatus());
  auto *func = dyn_vmcast<JSFunction>(*propRes);
  ASSERT_TRUE(func);

  auto *cb = func->getCodeBlock();
  ASSERT_TRUE(cb);

  std::vector<uint32_t> basicBlocks;
  llvh::DenseMap<uint32_t, unsigned> labels;

  discoverBasicBlocks(cb, basicBlocks, labels);
  EXPECT_EQ(6, basicBlocks.size());
  EXPECT_EQ(6, labels.size());
}

} // namespace
