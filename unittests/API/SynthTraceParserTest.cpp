/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/SynthTraceParser.h>

#include <gtest/gtest.h>

namespace {

using namespace facebook::hermes::tracing;

struct SynthTraceParserTest : public ::testing::Test {
  std::unique_ptr<llvh::MemoryBuffer> bufFromStr(const std::string &str) {
    llvh::StringRef ref{str.data(), str.size()};
    return llvh::MemoryBuffer::getMemBufferCopy(ref);
  }
};

TEST_F(SynthTraceParserTest, ParseHeader) {
  const char *src = R"(
{
  "version": 3,
  "globalObjID": 258,
  "runtimeConfig": {
    "gcConfig": {
      "minHeapSize": 1000,
      "initHeapSize": 33554432,
      "maxHeapSize": 536870912,
      "occupancyTarget": 0.75,
      "effectiveOOMThreshold": 20,
      "shouldReleaseUnused": "none",
      "name": "foo",
      "allocInYoung": false,
    },
    "maxNumRegisters": 100,
    "ES6Proxy": false,
    "Intl": false,
    "enableSampledStats": true,
    "vmExperimentFlags": 123
  },
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": []
}
)";
  auto result = parseSynthTrace(bufFromStr(src));
  const SynthTrace &trace = std::get<0>(result);
  hermes::vm::RuntimeConfig rtconf =
      std::get<1>(result).withGCConfig(std::get<2>(result).build()).build();

  EXPECT_EQ(trace.records().size(), 0);

  const ::hermes::vm::GCConfig &gcconf = rtconf.getGCConfig();
  EXPECT_EQ(gcconf.getMinHeapSize(), 1000);
  EXPECT_EQ(gcconf.getInitHeapSize(), 33554432);
  EXPECT_EQ(gcconf.getMaxHeapSize(), 536870912);
  EXPECT_EQ(gcconf.getOccupancyTarget(), 0.75);
  EXPECT_EQ(gcconf.getEffectiveOOMThreshold(), 20);
  EXPECT_FALSE(gcconf.getShouldReleaseUnused());
  EXPECT_EQ(gcconf.getName(), "foo");
  EXPECT_FALSE(gcconf.getAllocInYoung());

  EXPECT_EQ(rtconf.getMaxNumRegisters(), 100);
  EXPECT_FALSE(rtconf.getES6Proxy());
  EXPECT_FALSE(rtconf.getIntl());
  EXPECT_TRUE(rtconf.getEnableSampledStats());
  EXPECT_EQ(rtconf.getVMExperimentFlags(), 123);
}

TEST_F(SynthTraceParserTest, RuntimeConfigDefaults) {
  const char *src = R"(
{
  "version": 3,
  "globalObjID": 258,
  "runtimeConfig": {},
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": []
}
  )";
  auto result = parseSynthTrace(bufFromStr(src));
  hermes::vm::RuntimeConfig rtconf = std::get<1>(result).build();

  EXPECT_EQ(rtconf.getGCConfig().getMinHeapSize(), 0);
  EXPECT_EQ(rtconf.getGCConfig().getInitHeapSize(), 33554432);
  EXPECT_EQ(rtconf.getGCConfig().getMaxHeapSize(), 3221225472);
  EXPECT_FALSE(rtconf.getEnableSampledStats());
}

TEST_F(SynthTraceParserTest, SynthVersionMismatch) {
  const char *src = R"(
{
  "version": 0,
  "globalObjID": 258,
  "runtimeConfig": {
    "gcConfig": {
      "initHeapSize": 33554432,
      "maxHeapSize": 536870912
    }
  },
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": []
}
  )";
  ASSERT_THROW(parseSynthTrace(bufFromStr(src)), std::invalid_argument);
}

TEST_F(SynthTraceParserTest, SynthVersionInvalidKind) {
  const char *src = R"(
{
  "version": true,
  "globalObjID": 258,
  "runtimeConfig": {
    "gcConfig": {
      "initHeapSize": 33554432,
      "maxHeapSize": 536870912
    }
  },
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": []
}
  )";
  ASSERT_THROW(parseSynthTrace(bufFromStr(src)), std::invalid_argument);
}

TEST_F(SynthTraceParserTest, SynthMissingVersion) {
  const char *src = R"(
{
  "globalObjID": 258,
  "runtimeConfig": {
  },
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": []
}
  )";
  EXPECT_NO_THROW(parseSynthTrace(bufFromStr(src)));
}

} // namespace
