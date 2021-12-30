/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <hermes/CompileJS.h>
#include <hermes/hermes.h>

#include "SegmentTestCompile.h"

using namespace facebook::jsi;
using namespace facebook::hermes;

namespace {

TEST(SegmentTest, LoadSegmentTest) {
  std::shared_ptr<HermesRuntime> rt = makeHermesRuntime();

  std::string mainCode = R"(
    loadSegment(1, require.context);
    var foo = require('./foo.js');
    x = foo.x;
  )";

  std::string segmentCode = R"(
    exports.x = 42;
  )";

  auto code = hermes::genSplitCode(mainCode, segmentCode);

  auto mainBC = std::make_unique<StringBuffer>(std::move(code.first));
  auto segmentBC = std::make_unique<StringBuffer>(std::move(code.second));

  Function loadSegment = Function::createFromHostFunction(
      *rt,
      PropNameID::forAscii(*rt, "loadSegment"),
      2,
      [&segmentBC](
          Runtime &rt, const Value &thisVal, const Value *args, size_t count) {
        if (count < 2) {
          return Value::undefined();
        }
        static_cast<HermesRuntime &>(rt).loadSegment(
            std::move(segmentBC), args[1]);
        return Value::undefined();
      });
  rt->global().setProperty(*rt, "loadSegment", loadSegment);

  rt->evaluateJavaScript(std::move(mainBC), "main.js");
  ASSERT_EQ(rt->global().getProperty(*rt, "x").getNumber(), 42);
}

} // namespace
