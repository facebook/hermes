/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/SynthTraceParser.h>

#include <gtest/gtest.h>

#include <sstream>

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
  "version": 4,
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
  "version": 4,
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

TEST_F(SynthTraceParserTest, ParsePropID) {
  const char *src = R"(
{
  "version": 4,
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
  "trace": [
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 0,
      "propID": "propNameID:111",
      "propName": "prop111",
      "value": "number:0x0"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 0,
      "propID": "string:222",
      "propName": "prop222",
      "value": "number:0x0"
    }
  ]
}
  )";
  auto parseResult = parseSynthTrace(bufFromStr(src));
  SynthTrace &trace = std::get<0>(parseResult);

  auto record0 = dynamic_cast<const SynthTrace::GetPropertyRecord &>(
      *trace.records().at(0));
  ASSERT_EQ(SynthTrace::encodePropNameID(111), record0.propID_);

  auto record1 = dynamic_cast<const SynthTrace::GetPropertyRecord &>(
      *trace.records().at(1));
  ASSERT_EQ(SynthTrace::encodeString(222), record1.propID_);
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

TEST_F(SynthTraceParserTest, CreateBigIntRecord) {
  const char *src = R"(
{
  "globalObjID": 258,
  "runtimeConfig": {
  },
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": [
    {
      "type": "CreateBigIntRecord",
      "time": 0,
      "objID": 0,
      "method": "FromInt64",
      "bits": "999999999999"
    },
    {
      "type": "CreateBigIntRecord",
      "time": 0,
      "objID": 0,
      "method": "FromUint64",
      "bits": "123456789"
    }
  ]
}
  )";
  EXPECT_NO_THROW(parseSynthTrace(bufFromStr(src)));
}

TEST_F(SynthTraceParserTest, CreateBigIntRecordFailures) {
  auto createBigIntRecord = [](const char *record) -> std::string {
    std::stringstream src;

    src << R"(
{
  "globalObjID": 258,
  "runtimeConfig": {
  },
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": [
)" << record
        << R"(
  ]
}
  )";

    return src.str();
  };

  struct Test {
    const char *description;
    const char *createBigIntRecord;
  } shouldThrowInvalidArgument[] = {
      {"missing bits field",
       R"(
        {
          "type": "CreateBigIntRecord",
          "time": 0,
          "objID": 0,
          "method": "FromUint64"
        }
        )"},
      {"bits field is not a string",
       R"(
        {
          "type": "CreateBigIntRecord",
          "time": 0,
          "objID": 0,
          "method": "FromUint64",
          "bits": 123456
        }
        )"},
      {"bits has non-digit charaters",
       R"(
        {
          "type": "CreateBigIntRecord",
          "time": 0,
          "objID": 0,
          "method": "FromUint64",
          "bits": "123456!"
        })"},
      {"bits field is empty",
       R"(
        {
          "type": "CreateBigIntRecord",
          "time": 0,
          "objID": 0,
          "method": "FromUint64",
          "bits": ""
        }
        )"},
      {"bits is not a number",
       R"(
        {
          "type": "CreateBigIntRecord",
          "time": 0,
          "objID": 0,
          "method": "FromUint64",
          "bits": "I am not a number"
        })"},
  };

  for (const auto &test : shouldThrowInvalidArgument) {
    // TODO(T127739425): Enforce the type of this exception after we fix RTTI in
    // SynthTraceParser.
    EXPECT_ANY_THROW(parseSynthTrace(
        bufFromStr(createBigIntRecord(test.createBigIntRecord))))
        << test.description;
  }

  EXPECT_THROW(
      parseSynthTrace(bufFromStr(createBigIntRecord(R"(
        {
          "type": "CreateBigIntRecord",
          "time": 0,
          "objID": 0,
          "method": "FromUint64",
          "bits": "9999999999999999999999999999999999999999999"
        })"))),
      std::out_of_range)
      << "bits is out-of-range";
}

TEST_F(SynthTraceParserTest, BigIntToStringRecord) {
  const char *src = R"(
{
  "globalObjID": 258,
  "runtimeConfig": {
  },
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": [
    {
      "type": "BigIntToStringRecord",
      "time": 0,
      "strID": 0,
      "bigintID": 0,
      "radix": 20
    }
  ]
}
  )";
  EXPECT_NO_THROW(parseSynthTrace(bufFromStr(src)));
}

} // namespace
