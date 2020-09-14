/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMESVM_API_TRACE
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
    "ES6Symbol": false,
    "enableSampledStats": true,
    "vmExperimentFlags": 123
  },
  "env": {
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": []
}
)";
  auto result = parseSynthTrace(bufFromStr(src));
  const SynthTrace &trace = std::get<0>(result);
  const hermes::vm::RuntimeConfig &rtconf = std::get<1>(result).build();
  const hermes::vm::MockedEnvironment &env = std::get<3>(result);

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
  EXPECT_FALSE(rtconf.getES6Symbol());
  EXPECT_TRUE(rtconf.getEnableSampledStats());
  EXPECT_EQ(rtconf.getVMExperimentFlags(), 123);

  EXPECT_EQ(env.mathRandomSeed, 123);
  EXPECT_EQ(env.callsToDateNow.size(), 0);
  EXPECT_EQ(env.callsToNewDate.size(), 0);
  EXPECT_EQ(env.callsToDateAsFunction.size(), 0);
}

TEST_F(SynthTraceParserTest, RuntimeConfigDefaults) {
  const char *src = R"(
{
  "version": 3,
  "globalObjID": 258,
  "runtimeConfig": {},
  "env": {
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": []
}
  )";
  auto result = parseSynthTrace(bufFromStr(src));
  const hermes::vm::RuntimeConfig &rtconf = std::get<1>(result).build();

  EXPECT_EQ(rtconf.getGCConfig().getMinHeapSize(), 0);
  EXPECT_EQ(rtconf.getGCConfig().getInitHeapSize(), 33554432);
  EXPECT_EQ(rtconf.getGCConfig().getMaxHeapSize(), 536870912);
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
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
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
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
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
    "mathRandomSeed": 123,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": []
}
  )";
  EXPECT_NO_THROW(parseSynthTrace(bufFromStr(src)));
}

} // namespace

#endif
