/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#define DEBUG_TYPE "hbc-unittests"

#include "TestHelpers.h"

#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Runtime.h"

#include "gtest/gtest.h"

#include "llvm/Support/raw_ostream.h"

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
   ; while the sixth value is added through PutOwnById.
   ; This test makes sure that each value is correctly stored
   ; and that two objects can correctly use the same buffers.
   */

  const unsigned FRAME_SIZE = 4;
  BytecodeModuleGenerator BMG;
  auto BFG = BytecodeFunctionGenerator::create(BMG, FRAME_SIZE);

  // Need to generate the BytecodeModule twice.
  // Runtime needs to agree on string table contents with the bytecode,
  // which means the runtime must be initialized with a BytecodeModule.
  // First module just inserts the proper string indices into the runtime,
  // the second module actually generates instructions.
  std::vector<hermes::Literal *> objKeys{Builder.getLiteralString("a"),
                                         Builder.getLiteralString("b"),
                                         Builder.getLiteralString("c"),
                                         Builder.getLiteralString("d")};

  std::vector<hermes::Literal *> objVals{Builder.getLiteralBool(false),
                                         Builder.getLiteralBool(true),
                                         Builder.getLiteralNull(),
                                         Builder.getLiteralString("foo")};

  std::vector<hermes::Literal *> innerObjKeys{Builder.getLiteralString("bar")};
  std::vector<hermes::Literal *> innerObjVals{Builder.getLiteralNull()};

  auto objIdxs = BMG.addObjectBuffer(
      llvm::ArrayRef<hermes::Literal *>{objKeys},
      llvm::ArrayRef<hermes::Literal *>{objVals});
  auto innerObjIdxs = BMG.addObjectBuffer(
      llvm::ArrayRef<hermes::Literal *>{innerObjKeys},
      llvm::ArrayRef<hermes::Literal *>{innerObjVals});

  enum IDs { IDa = 6, IDb, IDc, IDd, IDe, IDfoo, IDbar };

  BFG->emitLoadConstInt(3, 1);
  BFG->emitNewObjectWithBuffer(0, 5, 4, objIdxs.first, objIdxs.second);
  BFG->emitNewObjectWithBuffer(
      1, 1, 1, innerObjIdxs.first, innerObjIdxs.second);
  BFG->emitPutOwnById(0, 1, IDe);

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

  auto *runtimeModule = RuntimeModule::create(
      runtime, BCProviderFromSrc::createBCProviderFromSrc(BMG.generate()));

  ASSERT_EQ(detail::mapString(*runtimeModule, "a"), IDa);
  ASSERT_EQ(detail::mapString(*runtimeModule, "b"), IDb);
  ASSERT_EQ(detail::mapString(*runtimeModule, "c"), IDc);
  ASSERT_EQ(detail::mapString(*runtimeModule, "d"), IDd);
  ASSERT_EQ(detail::mapString(*runtimeModule, "e"), IDe);
  ASSERT_EQ(detail::mapString(*runtimeModule, "foo"), IDfoo);
  ASSERT_EQ(detail::mapString(*runtimeModule, "bar"), IDbar);

  auto codeBlock = runtimeModule->getCodeBlock(0);

  CallResult<HermesValue> status{ExecutionStatus::EXCEPTION};
  {
    ScopedNativeCallFrame frame{
        runtime, 0, nullptr, false, HermesValue::encodeUndefinedValue()};
    status = runtime->interpretFunction(codeBlock);
  }
  auto frames = runtime->getStackFrames();
  ASSERT_TRUE(frames.begin() == frames.end());
  ASSERT_EQ(
      StackFrameLayout::CalleeExtraRegistersAtStart, runtime->getStackLevel());
  ASSERT_EQ(ExecutionStatus::RETURNED, status.getStatus());
  ASSERT_EQ(1, status.getValue().getDouble());
}

} // anonymous namespace
