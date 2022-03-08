/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "hbc-unittests"

#include "TestHelpers.h"

#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Runtime.h"

#include "gtest/gtest.h"

#include "llvh/Support/raw_ostream.h"

using namespace hermes::vm;
using namespace hermes::hbc;

namespace {

using ObjectBufferTest = RuntimeTestFixture;

TEST_F(ObjectBufferTest, TestNewObjectWithBuffer) {
  auto Ctx = std::make_shared<hermes::Context>();
  hermes::Module M(Ctx);
  hermes::IRBuilder Builder(&M);

  /*
   ; Store the object outlined below:
   ; obj = {
   ;         'a': false
   ;         'b': true
   ;         'c': null
   ;         'd': "foo"
   ;         'e': {"bar": null}
   ;       }
   ; First five values are added through NewObjectWithBuffer
   ; while the sixth value is added through PutNewOwnById.
   ; This test makes sure that each value is correctly stored
   ; and that two objects can correctly use the same buffers.
   */

  const unsigned FRAME_SIZE = 4;
  BytecodeModuleGenerator BMG;

  { // Make BMG aware of the strings it needs.
    UniquingStringLiteralAccumulator strings;

    // Name of the top-level function.
    strings.addString("global", /* isIdentifier */ false);

    // Strings used in the test.
    strings.addString("a", /* isIdentifier */ true);
    strings.addString("b", /* isIdentifier */ true);
    strings.addString("c", /* isIdentifier */ true);
    strings.addString("d", /* isIdentifier */ true);
    strings.addString("e", /* isIdentifier */ true);
    strings.addString("foo", /* isIdentifier */ false);
    strings.addString("bar", /* isIdentifier */ true);

    BMG.initializeStringTable(
        UniquingStringLiteralAccumulator::toTable(std::move(strings)));
  }

  auto BFG = BytecodeFunctionGenerator::create(BMG, FRAME_SIZE);

  // Need to generate the BytecodeModule twice.
  // Runtime needs to agree on string table contents with the bytecode,
  // which means the runtime must be initialized with a BytecodeModule.
  // First module just inserts the proper string indices into the runtime,
  // the second module actually generates instructions.
  std::vector<hermes::Literal *> objKeys{
      Builder.getLiteralString("a"),
      Builder.getLiteralString("b"),
      Builder.getLiteralString("c"),
      Builder.getLiteralString("d")};

  std::vector<hermes::Literal *> objVals{
      Builder.getLiteralBool(false),
      Builder.getLiteralBool(true),
      Builder.getLiteralNull(),
      Builder.getLiteralString("foo")};

  std::vector<hermes::Literal *> innerObjKeys{Builder.getLiteralString("bar")};
  std::vector<hermes::Literal *> innerObjVals{Builder.getLiteralNull()};

  auto objIdxs = BMG.addObjectBuffer(
      llvh::ArrayRef<hermes::Literal *>{objKeys},
      llvh::ArrayRef<hermes::Literal *>{objVals});
  auto innerObjIdxs = BMG.addObjectBuffer(
      llvh::ArrayRef<hermes::Literal *>{innerObjKeys},
      llvh::ArrayRef<hermes::Literal *>{innerObjVals});

  auto IDa = BMG.getStringID("a");
  auto IDb = BMG.getStringID("b");
  auto IDc = BMG.getStringID("c");
  auto IDd = BMG.getStringID("d");
  auto IDe = BMG.getStringID("e");
  auto IDfoo = BMG.getStringID("foo");
  auto IDbar = BMG.getStringID("bar");

  BFG->emitLoadConstInt(3, 1);
  BFG->emitNewObjectWithBuffer(0, 5, 4, objIdxs.first, objIdxs.second);
  BFG->emitNewObjectWithBuffer(
      1, 1, 1, innerObjIdxs.first, innerObjIdxs.second);
  BFG->emitPutNewOwnById(0, 1, IDe);

  BFG->emitGetById(1, 0, 0, IDa);
  BFG->emitLoadConstFalse(2);
  BFG->emitStrictEq(2, 1, 2);
  BFG->emitBitAnd(3, 2, 3);

  BFG->emitGetById(1, 0, 0, IDb);
  BFG->emitLoadConstTrue(2);
  BFG->emitStrictEq(2, 1, 2);
  BFG->emitBitAnd(3, 2, 3);

  BFG->emitGetById(1, 0, 0, IDc);
  BFG->emitLoadConstNull(2);
  BFG->emitStrictEq(2, 1, 2);
  BFG->emitBitAnd(3, 2, 3);

  BFG->emitGetById(1, 0, 0, IDd);
  BFG->emitLoadConstString(2, IDfoo);
  BFG->emitStrictEq(2, 1, 2);
  BFG->emitBitAnd(3, 2, 3);

  BFG->emitGetById(1, 0, 0, IDe);
  BFG->emitGetById(1, 1, 0, IDbar);
  BFG->emitLoadConstNull(2);
  BFG->emitStrictEq(2, 1, 2);
  BFG->emitBitAnd(3, 2, 3);

  BFG->emitRet(3);
  BFG->setHighestReadCacheIndex(255);
  BFG->setHighestWriteCacheIndex(255);
  BFG->bytecodeGenerationComplete();
  auto F = Builder.createTopLevelFunction(true);
  BMG.addFunction(F);
  BMG.setFunctionGenerator(F, std::move(BFG));

  auto *runtimeModule = RuntimeModule::createUninitialized(runtime, domain);

  runtimeModule->initializeWithoutCJSModulesMayAllocate(
      BCProviderFromSrc::createBCProviderFromSrc(BMG.generate()));

  auto codeBlock = runtimeModule->getCodeBlockMayAllocate(0);
  CallResult<HermesValue> status{ExecutionStatus::EXCEPTION};
  {
    ScopedNativeCallFrame frame{
        runtime,
        0,
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue()};
    status = runtime.interpretFunction(codeBlock);
  }
  auto frames = runtime.getStackFrames();
  ASSERT_TRUE(frames.begin() == frames.end());
  ASSERT_EQ(
      StackFrameLayout::CalleeExtraRegistersAtStart, runtime.getStackLevel());
  ASSERT_EQ(ExecutionStatus::RETURNED, status.getStatus());
  ASSERT_EQ(1, status.getValue().getDouble());
}

} // anonymous namespace
#undef DEBUG_TYPE
