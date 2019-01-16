#include "hermes/VM/JIT/DiscoverBB.h"

#include "hermes/VM/Callable.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/Runtime.h"

#include "../TestHelpers.h"
#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

TEST(DiscoverBBTest, SmokeTest) {
  Runtime runtime{kTestRTConfigLargeHeap};

  hermes::hbc::CompileFlags runFlags;
  runFlags.optimize = true;
  runtime.run(
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

  auto propRes =
      JSObject::getNamed(runtime.getGlobal(), &runtime, *benchSym.getValue());
  ASSERT_EQ(ExecutionStatus::RETURNED, propRes.getStatus());
  auto *func = dyn_vmcast<JSFunction>(*propRes);
  ASSERT_TRUE(func);

  auto *cb = func->getCodeBlock();
  ASSERT_TRUE(cb);

  std::vector<uint32_t> basicBlocks;
  llvm::DenseMap<uint32_t, unsigned> labels;

  discoverBasicBlocks(cb, basicBlocks, labels);
  EXPECT_EQ(6, basicBlocks.size());
  EXPECT_EQ(6, labels.size());
}

} // namespace
