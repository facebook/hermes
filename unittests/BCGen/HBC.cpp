/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "llvh/Support/raw_ostream.h"

#include "hermes/BCGen/HBC/BytecodeDataProvider.h"
#include "hermes/BCGen/HBC/BytecodeDisassembler.h"
#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/BCGen/HBC/BytecodeStream.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/HBC/Passes.h"
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/Public/Buffer.h"
#include "hermes/SourceMap/SourceMapGenerator.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

#include <initializer_list>

#define DEBUG_TYPE "hbc-unittests"

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

/// Create a string table appropriate (i.e. that contains all the necessary
/// strings) for use with these tests.  Additionally adds the strings in
/// \p strs if the test requires extra
StringLiteralTable stringsForTest(
    std::initializer_list<llvh::StringRef> strs = {}) {
  UniquingStringLiteralAccumulator strings;

  strings.addString("global", /* isIdentifier */ false);
  for (auto &str : strs) {
    strings.addString(str, /* isIdentifier */ false);
  }

  return UniquingStringLiteralAccumulator::toTable(std::move(strings));
}

TEST(HBCBytecodeGen, IntegrationTest) {
  std::string Result;
  llvh::raw_string_ostream OS(Result);

  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);

  Ctx->setDebugInfoSetting(DebugInfoSetting::ALL);

  BytecodeModuleGenerator BMG;
  BMG.initializeStringTable(stringsForTest({"f1"}));
  BMG.addFilename("main.js");

  Function *globalFunction = Builder.createTopLevelFunction({});
  auto BFG1 = BytecodeFunctionGenerator::create(BMG, 3);
  BFG1->emitMov(1, 2);
  BMG.setEntryPointIndex(BMG.addFunction(globalFunction));
  BMG.setFunctionGenerator(globalFunction, std::move(BFG1));

  Function *f1 =
      Builder.createFunction("f1", Function::DefinitionKind::ES5Function, true);
  auto BFG2 = BytecodeFunctionGenerator::create(BMG, 10);
  BFG2->setSourceLocation(DebugSourceLocation(0, 0, 1, 1, 0));
  const DebugSourceLocation debugSourceLoc(0, 1, 20, 300, 0);
  BFG2->addDebugSourceLocation(debugSourceLoc);
  BFG2->emitCall(9, 8, 7);
  BMG.addFunction(f1);
  BMG.setFunctionGenerator(f1, std::move(BFG2));

  BytecodeSerializer BS{OS};
  std::shared_ptr<BytecodeModule> BM = BMG.generate();
  BS.serialize(*BM, SHA1{});

  int globalFunctionIndex = BM->getGlobalFunctionIndex();
  int globalFunctionOffsetExpected =
      BM->getFunction(globalFunctionIndex).getOffset();
  EXPECT_GE(globalFunctionOffsetExpected, 0);

  auto bytecode = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
                      std::make_unique<StringBuffer>(OS.str()))
                      .first;

  int functionCnt = bytecode->getFunctionCount();
  EXPECT_EQ(functionCnt, 2);

  globalFunctionIndex = bytecode->getGlobalFunctionIndex();
  EXPECT_TRUE(globalFunctionIndex == 0 || globalFunctionIndex == 1);
  int globalFunctionOffsetActual =
      bytecode->getFunctionHeader(globalFunctionIndex).offset();
  EXPECT_EQ(globalFunctionOffsetExpected, globalFunctionOffsetActual);

  // We permit globalFunctionIndex to be 0 or 1; f1 must be the other value.
  int f1Index = 1 - globalFunctionIndex;
  const BytecodeFunction &oldF1 = BM->getFunction(f1Index);
  EXPECT_EQ(
      oldF1.getDebugOffsets()->sourceLocations,
      bytecode->getDebugOffsets(f1Index)->sourceLocations);
  EXPECT_EQ(
      oldF1.getDebugOffsets()->lexicalData,
      bytecode->getDebugOffsets(f1Index)->lexicalData);
  auto optionalSourceLoc = bytecode->getDebugInfo()->getLocationForAddress(
      bytecode->getDebugOffsets(f1Index)->sourceLocations, 0);
  EXPECT_TRUE(optionalSourceLoc.hasValue());
  EXPECT_EQ(*optionalSourceLoc, debugSourceLoc);

  // Verify basic properties of the source map.
  // We have two functions. Each function has one segment per debug location.
  SourceMapGenerator sourceMap;
  sourceMap.addSource("main.js");
  BM->populateSourceMap(&sourceMap);
  const auto &mappings = sourceMap.getMappingsLines();
  EXPECT_EQ(mappings.size(), 1u);
  EXPECT_EQ(mappings[0].size(), 2u);
}

TEST(HBCBytecodeGen, StripDebugInfo) {
  // Test that stripping debug info is successful.
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);

  Ctx->setDebugInfoSetting(DebugInfoSetting::ALL);

  BytecodeModuleGenerator BMG;
  BMG.initializeStringTable(stringsForTest());

  Function *globalFunction = Builder.createTopLevelFunction({});
  auto BFG1 = BytecodeFunctionGenerator::create(BMG, 3);
  BFG1->addDebugSourceLocation(DebugSourceLocation{0, 1, 20, 300, 0});
  BFG1->emitMov(1, 2);
  BMG.setEntryPointIndex(BMG.addFunction(globalFunction));
  BMG.setFunctionGenerator(globalFunction, std::move(BFG1));

  std::shared_ptr<BytecodeModule> BM = BMG.generate();

  BytecodeGenerationOptions opts = BytecodeGenerationOptions::defaults();

  std::string unstrippedStorage;
  llvh::raw_string_ostream unstrippedOS(unstrippedStorage);
  opts.stripDebugInfoSection = false;
  BytecodeSerializer{unstrippedOS, opts}.serialize(*BM, SHA1{});

  std::string strippedStorage;
  llvh::raw_string_ostream strippedOS(strippedStorage);
  opts.stripDebugInfoSection = true;
  BytecodeSerializer{strippedOS, opts}.serialize(*BM, SHA1{});

  // Stripping should reduce the size.
  EXPECT_LT(strippedOS.str().size(), unstrippedOS.str().size());

  // Verify debug info absent from decoded BM.
  std::unique_ptr<hbc::BCProvider> strippedBC =
      hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
          std::make_unique<StringBuffer>(strippedOS.str()))
          .first;
  ASSERT_EQ(strippedBC->getFunctionCount(), BM->getNumFunctions());
  for (uint32_t i = 0, max = BM->getNumFunctions(); i < max; i++) {
    EXPECT_TRUE(BM->getFunction(i).hasDebugInfo());
    EXPECT_FALSE(strippedBC->getFunctionHeader(i).flags().hasDebugInfo);
  }
}

TEST(HBCBytecodeGen, StringTableTest) {
  std::string Result;
  llvh::raw_string_ostream OS(Result);

  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);

  BytecodeModuleGenerator BMG;
  BMG.initializeStringTable(
      stringsForTest({"foo", "bar", /* Ā */ "\xc4\x80", /* å */ "\xc3\xa5"}));

  Function *F = Builder.createTopLevelFunction(true);
  auto BFG = BytecodeFunctionGenerator::create(BMG, 2);
  auto fooIdx1 = BFG->getStringID(Builder.getLiteralString("foo"));
  auto barIdx1 = BFG->getStringID(Builder.getLiteralString("bar"));
  auto fooIdx2 = BFG->getStringID(Builder.getLiteralString("foo"));
  auto barIdx2 = BFG->getStringID(Builder.getLiteralString("bar"));
  auto unicodeIdx1 =
      BFG->getStringID(Builder.getLiteralString("\xc4\x80")); // Ā
  auto unicodeIdx2 =
      BFG->getStringID(Builder.getLiteralString("\xc3\xa5")); // å

  BFG->emitLoadConstString(1, fooIdx1);
  BFG->emitLoadConstString(1, barIdx1);
  BFG->emitLoadConstString(1, fooIdx2);
  BFG->emitLoadConstString(1, barIdx2);
  BFG->emitLoadConstString(1, unicodeIdx1);
  BFG->emitLoadConstString(1, unicodeIdx2);
  BFG->bytecodeGenerationComplete();

  BMG.setEntryPointIndex(BMG.addFunction(F));
  BMG.setFunctionGenerator(F, std::move(BFG));
  std::shared_ptr<BytecodeModule> BM = BMG.generate();
  // 4 strings + function name.
  EXPECT_EQ(BM->getStringTableSize(), 5u);
  // function name "global" + 'foobar' + 2 chars per unicode char
  EXPECT_EQ(BM->getStringStorageSize(), 16u);

  Result.clear();
  BytecodeSerializer BS{OS};
  BS.serialize(*BM, SHA1{});

  auto bytecode = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
                      std::make_unique<StringBuffer>(OS.str()))
                      .first;

  EXPECT_EQ(bytecode->getStringCount(), 5u);
  EXPECT_EQ(bytecode->getStringStorage().size(), 16u);

  // bar
  EXPECT_EQ(bytecode->getStringTableEntry(0).getOffset(), 0u);
  EXPECT_EQ(bytecode->getStringTableEntry(0).getLength(), 3u);
  EXPECT_FALSE(bytecode->getStringTableEntry(0).isUTF16());

  // foo
  EXPECT_EQ(bytecode->getStringTableEntry(1).getOffset(), 3u);
  EXPECT_EQ(bytecode->getStringTableEntry(1).getLength(), 3u);
  EXPECT_FALSE(bytecode->getStringTableEntry(1).isUTF16());

  // global
  EXPECT_EQ(bytecode->getStringTableEntry(2).getOffset(), 6u);
  EXPECT_EQ(bytecode->getStringTableEntry(2).getLength(), 6u);
  EXPECT_FALSE(bytecode->getStringTableEntry(2).isUTF16());

  // UTF16
  EXPECT_EQ(bytecode->getStringTableEntry(3).getOffset(), 12u);
  EXPECT_EQ(bytecode->getStringTableEntry(3).getLength(), 1u);
  EXPECT_TRUE(bytecode->getStringTableEntry(3).isUTF16());

  // UTF16
  EXPECT_EQ(bytecode->getStringTableEntry(4).getOffset(), 14u);
  EXPECT_EQ(bytecode->getStringTableEntry(4).getLength(), 1u);
  EXPECT_TRUE(bytecode->getStringTableEntry(4).isUTF16());
}

TEST(HBCBytecodeGen, ExceptionTableTest) {
  std::string Result;
  llvh::raw_string_ostream OS(Result);

  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);

  BytecodeModuleGenerator BMG;
  BMG.initializeStringTable(stringsForTest());

  Function *F = Builder.createTopLevelFunction(true);
  auto BFG = BytecodeFunctionGenerator::create(BMG, 3);
  BFG->emitMov(1, 2);
  BFG->addExceptionHandler(HBCExceptionHandlerInfo{0, 10, 100});
  BFG->addExceptionHandler(HBCExceptionHandlerInfo{0, 20, 200});
  BFG->addExceptionHandler(HBCExceptionHandlerInfo{50, 60, 300});

  BMG.setEntryPointIndex(BMG.addFunction(F));
  BMG.setFunctionGenerator(F, std::move(BFG));

  std::shared_ptr<BytecodeModule> BM = BMG.generate();
  auto &BF = BM->getGlobalCode();
  ASSERT_EQ(BF.getExceptionHandlerCount(), 3u);

  BytecodeSerializer BS{OS};
  BS.serialize(*BM, SHA1{});

  auto bytecode = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
                      std::make_unique<StringBuffer>(OS.str()))
                      .first;

  ASSERT_EQ(bytecode->getExceptionTable(0).size(), 3u);
  EXPECT_EQ(bytecode->findCatchTargetOffset(0, 5), 100);
  EXPECT_EQ(bytecode->findCatchTargetOffset(0, 15), 200);
  EXPECT_EQ(bytecode->findCatchTargetOffset(0, 25), -1);
  EXPECT_EQ(bytecode->findCatchTargetOffset(0, 55), 300);
}

TEST(HBCBytecodeGen, ArrayBufferTest) {
  // Since deserialization of the array buffer now requires a codeblock,
  // the only thing that can be checked at BCGen time is that it uses
  // the proper number of bytes for the serialization format.
  std::string Result;
  llvh::raw_string_ostream OS(Result);

  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder Builder(&M);

  BytecodeModuleGenerator BMG;
  BMG.initializeStringTable(stringsForTest({"abc"}));

  Function *F = Builder.createTopLevelFunction(true);
  auto BFG = BytecodeFunctionGenerator::create(BMG, 3);
  BFG->emitMov(1, 2);
  std::vector<Literal *> arr1{
      Builder.getLiteralNumber(1),
      Builder.getLiteralBool(true),
      Builder.getLiteralBool(false),
      Builder.getLiteralNull(),
      Builder.getLiteralNull(),
      Builder.getLiteralString("abc"),
  };
  BMG.addArrayBuffer(llvh::ArrayRef<Literal *>{arr1});

  BMG.setEntryPointIndex(BMG.addFunction(F));
  BMG.setFunctionGenerator(F, std::move(BFG));

  std::shared_ptr<BytecodeModule> BM = BMG.generate();
  // auto &BF = BM->getGlobalCode();
  ASSERT_EQ(BM->getArrayBufferSize(), 10u);

  BytecodeSerializer BS{OS};
  BS.serialize(*BM, SHA1{});

  auto bytecode = hbc::BCProviderFromBuffer::createBCProviderFromBuffer(
                      std::make_unique<StringBuffer>(OS.str()))
                      .first;

  ASSERT_EQ(bytecode->getArrayBuffer().size(), 10u);
}

TEST(SpillRegisterTest, SpillsParameters) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder builder(&M);

  auto *F = builder.createTopLevelFunction(true);
  auto *BB = builder.createBasicBlock(F);
  builder.setInsertionBlock(BB);
  auto *undef = builder.getLiteralUndefined();

  // Create a 200 LoadConstInsts, requiring 200 registers
  std::vector<Value *> values;
  for (int i = 0; i < 200; i++) {
    values.push_back(builder.createHBCLoadConstInst(undef));
  }
  // Use them in a call to require 200 parameter registers.
  builder.createCallInst(undef, undef, values);
  builder.createReturnInst(undef);

  HVMRegisterAllocator RA(F);
  PostOrderAnalysis PO(F);
  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
  RA.allocate(order);

  PassManager PM;
  PM.addPass(new LowerCalls(RA));
  // Due to Mov elimination, many LoadConstInsts will be reallocated
  PM.addPass(new MovElimination(RA));
  PM.addPass(new SpillRegisters(RA));
  PM.run(F);

  // Ensure that spilling takes care of that
  for (auto &inst : *BB) {
    auto *load = llvh::dyn_cast<HBCLoadConstInst>(&inst);
    if (!load)
      continue;
    EXPECT_LT(RA.getRegister(load).getIndex(), 256u);
  }
}

TEST(SpillRegisterTest, NoStoreUnspilling) {
  auto Ctx = std::make_shared<Context>();
  Module M(Ctx);
  IRBuilder builder(&M);

  auto *F = builder.createTopLevelFunction(true);
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
  PostOrderAnalysis PO(F);
  llvh::SmallVector<BasicBlock *, 16> order(PO.rbegin(), PO.rend());
  RA.allocate(order);
  RA.allocateParameterCount(256);
  RA.updateRegister(store, Register(256));

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
  ASSERT_TRUE(fields.arrayBuffer.empty());
  ASSERT_TRUE(fields.objValueBuffer.empty());
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
  TestCompileFlags flags;
  flags.staticBuiltins = false;
  auto bytecodeVecDefault = bytecodeForSource("print('hello world');", flags);
  flags.staticBuiltins = true;
  auto bytecodeVecStaticBuiltins =
      bytecodeForSource("print('hello world');", flags);

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
