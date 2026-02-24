/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Optimizer/PassManager/PassManager.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"

#include "gtest/gtest.h"

using namespace hermes;

namespace {

class PassManagerTest : public ::testing::Test {
 public:
  PassManagerTest()
      : context_(std::make_shared<Context>(em_)),
        module_(context_),
        builder_(&module_),
        function_(builder_.createFunction(
            "main",
            Function::DefinitionKind::ES5Function,
            true)),
        block_(builder_.createBasicBlock(function_)) {
    builder_.setInsertionBlock(block_);
    module_.setTopLevelFunction(function_);
    builder_.createReturnInst(builder_.getLiteralEmpty());
    assert(!module_.getFunctionList().empty());
  }

  IRBuilder &getBuilder() {
    return builder_;
  }

  std::vector<llvh::StringRef> &getPassSeq() {
    return passSeq_;
  }

 protected:
  SourceErrorManager em_;
  std::shared_ptr<Context> context_;
  Module module_;
  IRBuilder builder_;
  Function *function_;
  BasicBlock *block_;

  std::vector<llvh::StringRef> passSeq_;

#define DefineTestPass(MorF, Name)                                \
  class Name : public MorF##Pass {                                \
   public:                                                        \
    /* \p changeIters is the number of applications of this pass  \
       instance that will cause a change. */                      \
    Name(                                                         \
        PassManagerTest *pmTest,                                  \
        unsigned maxChangeIters = 100,                            \
        unsigned changeIterInterval = 100)                        \
        : MorF##Pass(#Name),                                      \
          maxChangeIters_(maxChangeIters),                        \
          changeIterInterval_(changeIterInterval),                \
          pmTest_(pmTest) {                                       \
      assert(iters_ == 0);                                        \
    }                                                             \
    bool runOn##MorF(MorF *mf) override {                         \
      iters_++;                                                   \
      if ((iters_ % changeIterInterval_) == 0) {                  \
        return true;                                              \
      }                                                           \
      if (iters_ > maxChangeIters_) {                             \
        return true;                                              \
      }                                                           \
      pmTest_->getPassSeq().push_back(getName());                 \
      (void)pmTest_->getBuilder().createAllocObjectLiteralInst(); \
      return true;                                                \
    }                                                             \
                                                                  \
   private:                                                       \
    unsigned maxChangeIters_;                                     \
    unsigned changeIterInterval_;                                 \
    unsigned iters_ = 0;                                          \
    PassManagerTest *pmTest_;                                     \
  }

 protected:
  FunctionPass *getTestPass1(
      unsigned maxChangeIters = 100,
      unsigned changeIterInterval = 100) {
    DefineTestPass(Function, TestPass1);
    return new TestPass1(this, maxChangeIters, changeIterInterval);
  }
  ModulePass *getTestPass2(
      unsigned maxChangeIters = 100,
      unsigned changeIterInterval = 100) {
    DefineTestPass(Module, TestPass2);
    return new TestPass2(this, maxChangeIters, changeIterInterval);
  }
  FunctionPass *getTestPass3(
      unsigned maxChangeIters = 100,
      unsigned changeIterInterval = 100) {
    DefineTestPass(Function, TestPass3);
    return new TestPass3(this, maxChangeIters, changeIterInterval);
  }
};

using PassManagerDeathTest = PassManagerTest;

// These death tests rely on assertions firing, so they should
// only be run when assertions are enabled.
#ifndef NDEBUG
TEST_F(PassManagerDeathTest, UnterminatedFixedPointLoopTest0) {
  PassManager PM;
  PM.beginFixedPointLoop("test");
  EXPECT_DEATH_IF_SUPPORTED(
      PM.run(&module_), "FixedPointLoop left unterminated");
}

TEST_F(PassManagerDeathTest, UnterminatedFixedPointLoopTest1) {
  // More complicated version.
  PassManager PM;
  PM.addPass(getTestPass1());
  PM.beginFixedPointLoop("test-outer");
  PM.addPass(getTestPass1());
  PM.beginFixedPointLoop("test-inner");
  PM.addPass(getTestPass1());
  // Now terminate only one of the FixedPointLoops.
  EXPECT_DEATH_IF_SUPPORTED(
      PM.run(&module_), "FixedPointLoop left unterminated");
}

TEST_F(PassManagerDeathTest, TerminateLoopNotPresent0) {
  PassManager PM;
  PM.addPass(getTestPass1());
  EXPECT_DEATH_IF_SUPPORTED(
      PM.endFixedPointLoop(), "No FixedPointLoop to end.");
}

TEST_F(PassManagerDeathTest, TerminateLoopNotPresent1) {
  // More complicated version.
  PassManager PM;
  PM.addPass(getTestPass1());
  PM.beginFixedPointLoop("test");
  PM.addPass(getTestPass1());
  PM.endFixedPointLoop();
  // Second end has no matching begin.
  EXPECT_DEATH_IF_SUPPORTED(
      PM.endFixedPointLoop(), "No FixedPointLoop to end.");
}

TEST_F(PassManagerDeathTest, RunPipelineWithLoopOnFunction) {
  // More complicated version.
  PassManager PM;
  PM.addPass(getTestPass1());
  PM.beginFixedPointLoop("test");
  PM.addPass(getTestPass1());
  PM.endFixedPointLoop();
  // Second end has no matching begin.
  EXPECT_DEATH_IF_SUPPORTED(
      PM.run(function_), "Pipelines with loops may only be run with Modules.");
}
#endif // NDEBUG

TEST_F(PassManagerTest, SimpleLinearFunction) {
  PassManager PM;
  PM.addPass(getTestPass1());
  PM.addPass(getTestPass3());
  PM.run(function_);
  EXPECT_EQ(std::vector<llvh::StringRef>({"TestPass1", "TestPass3"}), passSeq_);
}

TEST_F(PassManagerTest, SimpleLinearModule) {
  PassManager PM;
  PM.addPass(getTestPass1());
  PM.addPass(getTestPass2());
  PM.addPass(getTestPass3());
  PM.run(&module_);
  EXPECT_EQ(
      std::vector<llvh::StringRef>({"TestPass1", "TestPass2", "TestPass3"}),
      passSeq_);
}

TEST_F(PassManagerTest, OneLevelLoopFinishInTime) {
  PassManager PM;
  PM.addPass(getTestPass1());
  PM.beginFixedPointLoop("test");
  PM.addPass(getTestPass2(3));
  PM.endFixedPointLoop();
  PM.addPass(getTestPass3());
  PM.run(&module_);
  EXPECT_EQ(
      std::vector<llvh::StringRef>(
          {"TestPass1", "TestPass2", "TestPass2", "TestPass2", "TestPass3"}),
      passSeq_);
}

TEST_F(PassManagerTest, OneLevelLoopExceedIterLimit) {
  PassManager PM("testPM");

  // Create a diagnostic handler to record the error messages.
  std::vector<std::string> messages;
  em_.setDiagHandler(
      [](const llvh::SMDiagnostic &diag, void *context) {
        auto *messages = static_cast<std::vector<std::string> *>(context);
        messages->push_back(diag.getMessage());
      },
      &messages);

  PM.beginFixedPointLoop("test");
  PM.addPass(getTestPass2(21));
  PM.endFixedPointLoop();
  PM.run(&module_);

  EXPECT_EQ(1, messages.size());
  const auto *kExpected =
      "Opt-to-fixed-point loop test reached max iterations (20).  "
      "This is likely a bug.";
  EXPECT_EQ(kExpected, messages.back());
}

TEST_F(PassManagerTest, ConsecutiveLoops) {
  PassManager PM;
  PM.beginFixedPointLoop("test0");
  PM.addPass(getTestPass1(3));
  PM.endFixedPointLoop();
  PM.beginFixedPointLoop("test1");
  PM.addPass(getTestPass2(2));
  PM.endFixedPointLoop();
  PM.run(&module_);
  EXPECT_EQ(
      std::vector<llvh::StringRef>(
          {"TestPass1", "TestPass1", "TestPass1", "TestPass2", "TestPass2"}),
      passSeq_);
}

TEST_F(PassManagerTest, NestedLoops) {
  PassManager PM;
  PM.addPass(getTestPass1());
  PM.beginFixedPointLoop("test-outer");
  const unsigned kOuterLoopIters = 3;
  PM.addPass(getTestPass2(kOuterLoopIters));
  PM.beginFixedPointLoop("test-inner");
  const unsigned kInnerLoopIters = 2;
  PM.addPass(
      getTestPass3(kOuterLoopIters * (kInnerLoopIters + 1), kOuterLoopIters));
  PM.endFixedPointLoop();
  PM.endFixedPointLoop();
  PM.addPass(getTestPass1());
  PM.run(&module_);
  EXPECT_EQ(
      std::vector<llvh::StringRef>(
          {"TestPass1",
           "TestPass2",
           "TestPass3",
           "TestPass3",
           "TestPass2",
           "TestPass3",
           "TestPass3",
           "TestPass2",
           "TestPass3",
           "TestPass3",
           "TestPass1"}),
      passSeq_);
}

} // anonymous namespace
