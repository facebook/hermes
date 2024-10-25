/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/raw_ostream.h"

#include "hermes/BCGen/HBC/BCProvider.h"
#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/HBC/SimpleBytecodeBuilder.h"
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/BCGen/Lowering.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/SourceMap/SourceMapGenerator.h"
#include "hermes/Support/Buffer.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

#include <initializer_list>

#define DEBUG_TYPE "hbc-unittests"

using llvh::dbgs;

using namespace hermes;
using namespace hermes::hbc;

namespace {

class StringBuffer : public Buffer {
 public:
  StringBuffer(std::string buffer) : string_(std::move(buffer)) {
    data_ = reinterpret_cast<const uint8_t *>(string_.c_str());
    size_ = string_.size();
  }

 private:
  std::string string_;
};

class VectorBuffer : public Buffer {
 public:
  VectorBuffer(std::vector<uint8_t> buffer) : vec_(std::move(buffer)) {
    data_ = reinterpret_cast<const uint8_t *>(vec_.data());
    size_ = vec_.size();
  }

 private:
  std::vector<uint8_t> vec_;
};

TEST(HBCBytecodeGen, ArrayBufferTest) {
  auto src = R"(
var arr = [1, true, false, null, null, 'abc']
)";
  auto BM = bytecodeModuleForSource(src);
  ASSERT_EQ(BM->getLiteralValueBufferSize(), 10u);
}

TEST(HBCBytecodeGen, ObjectBufferTest) {
  auto src = R"(
var obj = {a:1, b:2, c:3};
)";
  // Need to optimize to populate the object buffer.
  auto BM = bytecodeModuleForSource(
      src, BytecodeGenerationOptions::defaults(), /* optimize */ true);
  ASSERT_EQ(BM->getObjectKeyBufferSize(), 4u);
  ASSERT_EQ(BM->getLiteralValueBufferSize(), 13u);
}

TEST(HBCBytecodeGen, SmallFuncHeaderOffsetTest) {
  // Ensure the roundtrip survives.
  SmallFuncHeader small{0xdeadbeef};
  ASSERT_EQ(small.getLargeHeaderOffset(), 0xdeadbeef);
}

// For the following 'easy' tests, exact duplicate literals are used. These will
// always be de-duplicated, so we don't have to turn optimizations on.
TEST(HBCBytecodeGen, EasyArrayDedupBufferTest) {
  auto singleArrCode = R"(
var arr = [1, null, undefined, 9n, 'hi', {}, Symbol('sym')];
)";
  auto dupArrCode = R"(
var arr1 = [1, null, undefined, 9n, 'hi', {}, Symbol('sym')];
var arr2 = [1, null, undefined, 9n, 'hi', {}, Symbol('sym')];
var arr3 = [1, null, undefined, 9n, 'hi', {}, Symbol('sym')];
var arr4 = [1, null, undefined, 9n, 'hi', {}, Symbol('sym')];
var s = arr1[0] + arr2[0] + arr3[0] + arr4[0];
)";
  auto singleArrBM = bytecodeModuleForSource(singleArrCode);
  auto manyArrBM = bytecodeModuleForSource(dupArrCode);
  // If de-duplication is working correctly, the amount of space that a single
  // array literal takes up should be the same amount of space that N identical
  // array literals take up.
  ASSERT_EQ(
      singleArrBM->getLiteralValueBufferSize(),
      manyArrBM->getLiteralValueBufferSize());
}

TEST(HBCBytecodeGen, EasyObjectDedupBufferTest) {
  auto singleObjCode = R"(
var obj = {a:1, b:null, c:undefined, d:9n, e:'hi', f:{}, g:Symbol('sym')};
)";
  auto dupObjsCode = R"(
var obj1 = {a:1, b:null, c:undefined, d:9n, e:'hi', f:{}, g:Symbol('sym')};
var obj2 = {a:1, b:null, c:undefined, d:9n, e:'hi', f:{}, g:Symbol('sym')};
var obj3 = {a:1, b:null, c:undefined, d:9n, e:'hi', f:{}, g:Symbol('sym')};
var obj4 = {a:1, b:null, c:undefined, d:9n, e:'hi', f:{}, g:Symbol('sym')};
var s = obj1.a + obj2.a + obj3.a + obj4.a;
)";
  auto singleObjBM = bytecodeModuleForSource(singleObjCode);
  auto dedupBM = bytecodeModuleForSource(dupObjsCode);
  // If de-duplication is working correctly, the amount of space that a single
  // object literal takes up should be the same amount of space that N identical
  // object literals take up.
  ASSERT_EQ(
      singleObjBM->getObjectKeyBufferSize(), dedupBM->getObjectKeyBufferSize());
  ASSERT_EQ(
      singleObjBM->getLiteralValueBufferSize(),
      dedupBM->getLiteralValueBufferSize());
}

// For the next 'hard' tests, the literals are not exact duplicates. If
// optimizations are off, we will simply give up there and not perform any
// de-duplications. However, if optimizations are on, then we perform a more
// advanced algorithm for de-duplicating, which will result in a smaller buffer
// size.
TEST(HBCBytecodeGen, HardArrayDedupBufferTest) {
  auto almostDupCode = R"(
var arr1 = [false,                  true, null, 'hi', null, true, false, 'bye'];
var arr2 = ['diff', 4, 5, 6, false, true, null, 'hi', null, true, false, 'bye'];
var s = arr1[0] + arr2[1];
)";
  auto dupBM = bytecodeModuleForSource(almostDupCode);
  auto dedupOpts = BytecodeGenerationOptions::defaults();
  dedupOpts.optimizationEnabled = true;
  auto dedupBM = bytecodeModuleForSource(almostDupCode, dedupOpts);
  // The bytecode module which performed optimizations should have a smaller
  // buffer size than the one that didn't.
  ASSERT_LT(
      dedupBM->getLiteralValueBufferSize(), dupBM->getLiteralValueBufferSize());
}

TEST(HBCBytecodeGen, HardObjectDedupBufferTest) {
  auto almostDupCode = R"(
var obj1 = {a:10,b:11,c:12,1:null,2:true,3:false};
var obj2 = {a:10,b:11,c:12};
var s = obj1.a + obj2.a;
)";
  auto dedupOpts = BytecodeGenerationOptions::defaults();
  auto dupBM =
      bytecodeModuleForSource(almostDupCode, dedupOpts, /* optimize */ true);
  dedupOpts.optimizationEnabled = true;
  auto dedupBM =
      bytecodeModuleForSource(almostDupCode, dedupOpts, /* optimize */ true);
  // The bytecode module which performed optimizations should have a smaller
  // buffer size than the one that didn't.
  ASSERT_LT(dedupBM->getObjectKeyBufferSize(), dupBM->getObjectKeyBufferSize());
  ASSERT_LT(
      dedupBM->getLiteralValueBufferSize(), dupBM->getLiteralValueBufferSize());
}

TEST(SpillRegisterTest, SpillsParameters) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder builder(&M);

  auto *F = builder.createTopLevelFunction("global", true);
  auto *BB = builder.createBasicBlock(F);
  builder.setInsertionBlock(BB);
  auto *undef = builder.getLiteralUndefined();
  auto *emptySen = builder.getEmptySentinel();

  // Create a 200 LoadConstInsts, requiring 200 registers
  std::vector<Value *> values;
  for (int i = 0; i < 200; i++) {
    values.push_back(builder.createHBCLoadConstInst(undef));
  }
  // Use them in a call to require 200 parameter registers.
  builder.createCallInst(
      undef, emptySen, false, emptySen, undef, undef, values);
  builder.createReturnInst(undef);

  HVMRegisterAllocator RA(F);
  auto PO = postOrderAnalysis(F);
  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
  RA.allocate(order);

  PassManager PM;
  PM.addPass(new LowerCalls());
  // Due to Mov elimination, many LoadConstInsts will be reallocated
  PM.addPass(new MovElimination(RA));
  PM.addPass(new SpillRegisters(RA));
  PM.run(F);

  // Ensure that spilling takes care of that
  for (auto &inst : *BB) {
    auto *load = llvh::dyn_cast<HBCLoadConstInst>(&inst);
    if (!load)
      continue;
    EXPECT_LT(RA.getRegister(load).getIndexInClass(), 256u);
  }
}

TEST(SpillRegisterTest, NoStoreUnspilling) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder builder(&M);

  auto *F = builder.createTopLevelFunction("global", true);
  auto *BB = builder.createBasicBlock(F);
  builder.setInsertionBlock(BB);

  // Create a simple store
  auto *store = builder.createStorePropertyInst(
      builder.getLiteralUndefined(),
      builder.getLiteralUndefined(),
      builder.getLiteralUndefined());
  builder.createReturnInst(builder.getLiteralUndefined());

  // Allocate that store to a high register
  HVMRegisterAllocator RA(F);
  auto PO = postOrderAnalysis(F);
  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
  RA.allocate(order);
  RA.allocateParameterCount(256);
  RA.updateRegister(store, Register(RegClass::Other, 256));

  // Ensure that spilling doesn't insert any additional instructions
  unsigned sizeBefore = BB->size();
  PassManager PM;
  PM.addPass(new SpillRegisters(RA));
  PM.run(F);
  EXPECT_EQ(sizeBefore, BB->size());
}

TEST(HBCBytecodeGen, BytecodeFields) {
  std::string error;
  auto bytecode = bytecodeForSource("print('Hello World');");
  ConstBytecodeFileFields fields;
  bool populated = fields.populateFromBuffer(bytecode, &error);
  ASSERT_TRUE(populated);
  ASSERT_TRUE(error.empty());
  EXPECT_EQ(fields.header->magic, hbc::MAGIC);
  ASSERT_EQ(fields.functionHeaders.size(), 1); // global function
  // Three functions:  print, 'Hello World', file name
  ASSERT_EQ(fields.stringTableEntries.size(), 3);
  ASSERT_TRUE(fields.literalValueBuffer.empty());
  ASSERT_TRUE(fields.regExpTable.empty());
  ASSERT_TRUE(fields.regExpStorage.empty());
}

TEST(HBCBytecodeGen, BytecodeFieldsFail) {
  std::string error;
  std::vector<uint8_t> bytecode = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  ConstBytecodeFileFields fields;
  bool populated = fields.populateFromBuffer(bytecode, &error);
  EXPECT_FALSE(populated);
  EXPECT_FALSE(error.empty());
}

TEST(HBCBytecodeGen, SerializeBytecodeOptions) {
  BytecodeGenerationOptions opts = BytecodeGenerationOptions::defaults();
  opts.staticBuiltinsEnabled = false;
  auto bytecodeVecDefault = bytecodeForSource("print('hello world');", opts);
  opts.staticBuiltinsEnabled = true;
  auto bytecodeVecStaticBuiltins =
      bytecodeForSource("print('hello world');", opts);

  auto bytecodeDefault = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
                             std::make_unique<VectorBuffer>(bytecodeVecDefault))
                             .first;
  auto bytecodeStaticBuiltins =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
          std::make_unique<VectorBuffer>(bytecodeVecStaticBuiltins))
          .first;
  ASSERT_TRUE(bytecodeDefault);
  ASSERT_TRUE(bytecodeStaticBuiltins);
  EXPECT_FALSE(bytecodeDefault->getBytecodeOptions().staticBuiltins);
  EXPECT_TRUE(bytecodeStaticBuiltins->getBytecodeOptions().staticBuiltins);
}

TEST(HBCBytecodeGen, BytecodeOptionHasAsync) {
  auto bytecodeVecNoAsync = bytecodeForSource("function foo(){}");
  auto bytecodeVecHasAsync = bytecodeForSource("async function foo(){}");

  auto bytecodeNoAsync = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
                             std::make_unique<VectorBuffer>(bytecodeVecNoAsync))
                             .first;
  auto bytecodeHasAsync =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
          std::make_unique<VectorBuffer>(bytecodeVecHasAsync))
          .first;
  ASSERT_TRUE(bytecodeNoAsync);
  ASSERT_TRUE(bytecodeHasAsync);
  EXPECT_FALSE(bytecodeNoAsync->getBytecodeOptions().hasAsync);
  EXPECT_TRUE(bytecodeHasAsync->getBytecodeOptions().hasAsync);
}

} // end anonymous namespace
#undef DEBUG_TYPE
