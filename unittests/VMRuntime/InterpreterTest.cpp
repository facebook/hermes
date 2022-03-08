/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"

#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StringView.h"

#include "gtest/gtest.h"

#include "llvh/Support/raw_ostream.h"

using namespace hermes::vm;
using namespace hermes::hbc;

/// Associate a label with an instruction. Use it like this:
/// \begincode
///   LABEL(L(1), builder.emitMov(1, 2));
/// \endcode
#define LABEL(L, emit) \
  do {                 \
    auto ofs = emit;   \
    if (pass == 0)     \
      labels[L] = ofs; \
  } while (0)

/// Emit a conditional jump to label. Use it like this:
/// \begincode
///   JCOND(builder.emitJLessEqualN, L(2), 0, 2);
/// \endcode
#define JCOND(name, L, op2, op3)              \
  do {                                        \
    int line = __LINE__;                      \
    if (pass == 0)                            \
      jmps[line] = name(0, op2, op3);         \
    else                                      \
      name(labels[L] - jmps[line], op2, op3); \
  } while (0);

/// Emit an unconditional jump to label. Use it like this:
/// \begincode
///   JMP(builder.emitJmp, L(2));
/// \endcode
#define JMP(name, L)                \
  do {                              \
    int line = __LINE__;            \
    if (pass == 0)                  \
      jmps[line] = name(0);         \
    else                            \
      name(labels[L] - jmps[line]); \
  } while (0);

/// Make labels more noticeable in the source by always using this macro to
/// refer to them.
#define L(x) x

namespace {

/// Convert all arguments to string and print them followed by new line.
static CallResult<HermesValue>
print(void *, Runtime &runtime, NativeArgs args) {
  GCScope scope(runtime);
  bool first = true;

  for (Handle<> arg : args.handles()) {
    auto res = toString_RJS(runtime, arg);
    if (res != ExecutionStatus::RETURNED)
      return ExecutionStatus::EXCEPTION;

    if (!first)
      llvh::outs() << " ";
    SmallU16String<32> tmp;
    llvh::outs() << StringPrimitive::createStringView(
                        runtime, runtime.makeHandle(std::move(*res)))
                        .getUTF16Ref(tmp);
    first = false;
  }

  llvh::outs() << "\n";
  return HermesValue::encodeUndefinedValue();
}

class InterpreterFunctionTest : public RuntimeTestFixture {
 private:
  Handle<Domain> domain;
  RuntimeModule *runtimeModule;
  BytecodeModuleGenerator BMG;
  bool hasRun = false;
  CallResult<HermesValue> result{ExecutionStatus::EXCEPTION};

 protected:
  std::unique_ptr<BytecodeFunctionGenerator> BFG;

  InterpreterFunctionTest()
      : RuntimeTestFixture(),
        domain(runtime.makeHandle(Domain::create(runtime))),
        runtimeModule(RuntimeModule::createUninitialized(runtime, domain)) {
    BFG = BytecodeFunctionGenerator::create(BMG, 1);
  }

  CallResult<HermesValue> run() {
    assert(!hasRun);
    BFG->bytecodeGenerationComplete();
    auto *codeBlock = createCodeBlock(runtimeModule, runtime, BFG.get());
    ScopedNativeCallFrame frame(
        runtime,
        0,
        HermesValue::encodeNativePointer(codeBlock),
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue());
    assert(!frame.overflowed());
    result = runtime.interpretFunction(codeBlock);

    hasRun = true;
    return result;
  }

  bool getResultAsBool() const {
    assert(hasRun);
    assert(result != ExecutionStatus::EXCEPTION);
    return result->getBool();
  }

  Handle<StringPrimitive> getResultAsString() {
    assert(hasRun);
    assert(result != ExecutionStatus::EXCEPTION);
    return runtime.makeHandle(result->getString());
  }
};

using InterpreterTest = RuntimeTestFixture;

TEST_F(InterpreterTest, SimpleSmokeTest) {
  auto *runtimeModule = RuntimeModule::createUninitialized(runtime, domain);

  /*
   ; calculate 10 - 2 and print "result =" the value.
   load_imm     reg0, 10
   load_imm     reg1, 2
   subn         reg2, reg0, reg1
   get_global   reg0
   get_named    reg1, reg0, "print"
   load_imm     reg3, #undefined
   push         reg3
   load_string  reg3, "result="
   push         reg3
   push         reg2
   call         reg3, reg1, 3
   ret          reg2
   */

  StringID printID = 1;
  StringID resultID = 2;

  const unsigned FRAME_SIZE = 16;
  BytecodeModuleGenerator BMG;
  auto BFG = BytecodeFunctionGenerator::create(BMG, FRAME_SIZE);

  BFG->emitLoadConstDoubleDirect(0, 10);
  BFG->emitLoadConstDoubleDirect(1, 2);
  BFG->emitSubN(2, 0, 1);
  BFG->emitGetGlobalObject(0);
  BFG->emitGetById(1, 0, 1, printID);
  BFG->emitLoadConstUndefined(3);
  BFG->emitMov(FRAME_SIZE + StackFrameLayout::ThisArg, 3);
  BFG->emitLoadConstString(FRAME_SIZE + StackFrameLayout::FirstArg, resultID);
  BFG->emitMov(FRAME_SIZE + StackFrameLayout::FirstArg - 1, 2);
  BFG->emitCall(3, 1, 3);
  BFG->emitRet(2);
  BFG->setHighestReadCacheIndex(1);
  BFG->setHighestWriteCacheIndex(0);

  BFG->bytecodeGenerationComplete();
  auto codeBlock = createCodeBlock(runtimeModule, runtime, BFG.get());

  ASSERT_EQ(detail::mapStringMayAllocate(*runtimeModule, "print"), printID);
  ASSERT_EQ(detail::mapStringMayAllocate(*runtimeModule, "result="), resultID);

  auto printFn = runtime.makeHandle<NativeFunction>(
      *NativeFunction::createWithoutPrototype(
          runtime,
          nullptr,
          print,
          Predefined::getSymbolID(Predefined::emptyString),
          0));

  // Define the 'print' function.
  (void)JSObject::putNamed_RJS(
      runtime.getGlobal(),
      runtime,
      runtimeModule->getSymbolIDFromStringIDMayAllocate(printID),
      printFn);

  CallResult<HermesValue> status{ExecutionStatus::EXCEPTION};
  {
    ScopedNativeCallFrame frame(
        runtime,
        0,
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue());
    ASSERT_FALSE(frame.overflowed());
    status = runtime.interpretFunction(codeBlock);
  }

  auto frames = runtime.getStackFrames();
  ASSERT_TRUE(frames.begin() == frames.end());
  ASSERT_EQ(
      StackFrameLayout::CalleeExtraRegistersAtStart, runtime.getStackLevel());
  ASSERT_EQ(ExecutionStatus::RETURNED, status.getStatus());
  ASSERT_EQ(8.0, status.getValue().getDouble());
}

TEST_F(InterpreterTest, IterativeFactorialTest) {
  auto runtimeModule = RuntimeModule::createUninitialized(runtime, domain);

  /*
   get_arg    reg0, 1           ; load n
   mov        reg1, reg0        ; res = n
   load_imm   reg2, 1           ; constant for reuse
   to_number  reg0, reg0        ; --n
L1:
   subn       reg0, reg0, reg2
   jlen       L2, reg0, reg2    ; if n <= 1 goto L2
   mul        reg1, reg1, reg0  ; res *= n
   jmp        L1
L2:
   ret        reg1              ; return res
   */

  std::map<int, int> labels{};
  std::map<int, int> jmps{};

  auto emit = [&](BytecodeFunctionGenerator &builder, int pass) {
    builder.emitLoadParam(0, 1);
    builder.emitMov(1, 0);
    builder.emitLoadConstDoubleDirect(2, 1);
    builder.emitToNumber(0, 0);
    LABEL(L(1), builder.emitSubN(0, 0, 2));
    JCOND(builder.emitJLessEqualN, L(2), 0, 2);
    builder.emitMul(1, 1, 0);
    JMP(builder.emitJmp, L(1));
    LABEL(L(2), builder.emitRet(1));
  };

  // Pass 0 - resolve labels.
  {
    BytecodeModuleGenerator BMG;
    auto BFG = BytecodeFunctionGenerator::create(BMG, 3);
    emit(*BFG, 0);
  }

  // Pass 1 - build the actual code.
  BytecodeModuleGenerator BMG;
  auto BFG = BytecodeFunctionGenerator::create(BMG, 3);
  emit(*BFG, 1);
  BFG->bytecodeGenerationComplete();
  auto codeBlock = createCodeBlock(runtimeModule, runtime, BFG.get());

  CallResult<HermesValue> status{ExecutionStatus::EXCEPTION};
  {
    ScopedNativeCallFrame newFrame(
        runtime,
        1,
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue(),
        HermesValue::encodeUndefinedValue());
    ASSERT_FALSE(newFrame.overflowed());
    newFrame->getArgRef(0) = HermesValue::encodeDoubleValue(5);
    status = runtime.interpretFunction(codeBlock);
  }
  auto frames = runtime.getStackFrames();
  ASSERT_TRUE(frames.begin() == frames.end());
  ASSERT_EQ(
      StackFrameLayout::CalleeExtraRegistersAtStart, runtime.getStackLevel());
  ASSERT_EQ(ExecutionStatus::RETURNED, status.getStatus());
  ASSERT_EQ(120.0, status.getValue().getDouble());
}

TEST_F(InterpreterTest, RecursiveFactorialTest) {
  auto runtimeModule = RuntimeModule::createUninitialized(runtime, domain);

  auto factID = detail::mapStringMayAllocate(*runtimeModule, "fact");

  /*
   get_arg    reg0, 1           ; load n
   load_imm   reg1, 2           ; load constant 2
   jg         L1, reg0, reg1    ; if n > 2 goto L1
   ret        reg0              ; return n
L1:
   load_imm   reg1, 1           ; load constant 1
   sub        reg2, reg0, reg1  ; reg2 = n-1
   load_imm   reg1, #undefined  ; fact(n-1)
   push       reg1
   push       reg2
   get_global reg1
   get_named  reg1, reg1, "fact"
   call       reg1, reg1, 2
   mul        reg0, reg0, reg1 ; return n*fact(n-1)
   ret        reg0
   */

  std::map<int, int> labels{};
  std::map<int, int> jmps{};
  const unsigned FRAME_SIZE = 16;

  auto emit = [&](BytecodeFunctionGenerator &builder, int pass) {
    builder.emitLoadParam(0, 1);
    builder.emitLoadConstDoubleDirect(1, 2);
    JCOND(builder.emitJGreater, L(1), 0, 1);
    builder.emitRet(0);
    LABEL(L(1), builder.emitLoadConstDoubleDirect(1, 1));
    builder.emitLoadConstUndefined(FRAME_SIZE + StackFrameLayout::ThisArg);
    builder.emitSub(FRAME_SIZE + StackFrameLayout::FirstArg, 0, 1);
    builder.emitGetGlobalObject(1);
    builder.emitGetById(1, 1, 0, factID);
    builder.emitCall(1, 1, 2);
    builder.emitMul(0, 0, 1);
    builder.emitRet(0);
  };

  // Pass 0 - resolve labels.
  {
    BytecodeModuleGenerator BMG;
    auto BFG = BytecodeFunctionGenerator::create(BMG, FRAME_SIZE);
    emit(*BFG, 0);
  }

  // Pass 1 - build the actual code.
  BytecodeModuleGenerator BMG;
  auto BFG = BytecodeFunctionGenerator::create(BMG, FRAME_SIZE);
  emit(*BFG, 1);
  BFG->setHighestReadCacheIndex(255);
  BFG->setHighestWriteCacheIndex(255);
  BFG->bytecodeGenerationComplete();
  auto codeBlock = createCodeBlock(runtimeModule, runtime, BFG.get());

  Handle<JSFunction> factFn = runtime.makeHandle(JSFunction::create(
      runtime,
      runtimeModule->getDomain(runtime),
      runtime.makeNullHandle<JSObject>(),
      runtime.makeNullHandle<Environment>(),
      codeBlock));

  // Define the 'fact' function.
  (void)JSObject::putNamed_RJS(
      runtime.getGlobal(),
      runtime,
      runtimeModule->getSymbolIDFromStringIDMayAllocate(factID),
      factFn);

  {
    CallResult<HermesValue> status{ExecutionStatus::EXCEPTION};
    {
      ScopedNativeCallFrame newFrame(
          runtime,
          1,
          HermesValue::encodeUndefinedValue(),
          HermesValue::encodeUndefinedValue(),
          HermesValue::encodeUndefinedValue());
      ASSERT_FALSE(newFrame.overflowed());
      newFrame->getArgRef(0) = HermesValue::encodeDoubleValue(2);
      status = runtime.interpretFunction(codeBlock);
    }
    auto frames = runtime.getStackFrames();
    ASSERT_TRUE(frames.begin() == frames.end());
    ASSERT_EQ(
        StackFrameLayout::CalleeExtraRegistersAtStart, runtime.getStackLevel());
    ASSERT_EQ(ExecutionStatus::RETURNED, status.getStatus());
    ASSERT_EQ(2.0, status.getValue().getDouble());
  }

  {
    CallResult<HermesValue> status{ExecutionStatus::EXCEPTION};
    {
      ScopedNativeCallFrame newFrame(
          runtime,
          1,
          HermesValue::encodeUndefinedValue(),
          HermesValue::encodeUndefinedValue(),
          HermesValue::encodeUndefinedValue());
      ASSERT_FALSE(newFrame.overflowed());
      newFrame->getArgRef(0) = HermesValue::encodeDoubleValue(5);
      status = runtime.interpretFunction(codeBlock);
    }
    auto frames = runtime.getStackFrames();
    ASSERT_TRUE(frames.begin() == frames.end());
    ASSERT_EQ(
        StackFrameLayout::CalleeExtraRegistersAtStart, runtime.getStackLevel());
    ASSERT_EQ(ExecutionStatus::RETURNED, status.getStatus());
    ASSERT_EQ(120.0, status.getValue().getDouble());
  }
}

TEST_F(InterpreterFunctionTest, GetByIdSlowPathChecksForExceptions) {
  BFG->emitLoadConstUndefined(0);
  BFG->emitGetById(0, 0, 0, 0);
  BFG->emitRet(0);
  ASSERT_EQ(ExecutionStatus::EXCEPTION, run());
}

TEST_F(InterpreterFunctionTest, PutByIdSlowPathChecksForExceptions) {
  BFG->emitLoadConstUndefined(0);
  BFG->emitPutById(0, 0, 0, 0);
  BFG->emitRet(0);
  ASSERT_EQ(ExecutionStatus::EXCEPTION, run());
}

TEST_F(InterpreterFunctionTest, TestNot) {
  BFG->emitLoadConstFalse(0);
  BFG->emitNot(0, 0);
  BFG->emitRet(0);
  ASSERT_EQ(ExecutionStatus::RETURNED, run());
  EXPECT_TRUE(getResultAsBool());
}

TEST_F(InterpreterFunctionTest, TestToString) {
  BFG->emitLoadConstFalse(0);
  BFG->emitAddEmptyString(0, 0);
  BFG->emitRet(0);
  ASSERT_EQ(ExecutionStatus::RETURNED, run());
  SmallU16String<8> tmp;
  getResultAsString()->appendUTF16String(tmp);
  ASSERT_EQ(createUTF16Ref(u"false"), tmp.arrayRef());
}

#if defined(NDEBUG) && !defined(HERMES_UBSAN) && \
    !LLVM_THREAD_SANITIZER_BUILD && !LLVM_ADDRESS_SANITIZER_BUILD
// Returns the native stack pointer of the callee frame.
static CallResult<HermesValue>
getSP(void *, Runtime &runtime, NativeArgs args) {
  int dummy;
  return HermesValue::encodeNativePointer(&dummy);
}

// Use a non-inline function to perform the stack size measurement so that it
// takes place in a new stack frame. This ensures that location of dummy on the
// stack really is right before the stack frame for interpretFunction.
LLVM_ATTRIBUTE_NOINLINE static void testInterpreterStackSize(
    Runtime &runtime,
    CodeBlock *codeBlock) {
  // Check that inner and outer stack pointer differ by at most a set threshold.
  int dummy;
  const auto outerStackPointer = reinterpret_cast<uintptr_t>(&dummy);
  auto status = runtime.interpretFunction(codeBlock);
  ASSERT_EQ(ExecutionStatus::RETURNED, status.getStatus());
  const auto innerStackPointer =
      reinterpret_cast<uintptr_t>(status.getValue().getNativePointer<void>());
  // Increase this only if you have a reason to grow the interpreter's frame.
#ifdef _MSC_VER
  // TODO(T42117517) Understand why stack frame size is large on Windows
  uintptr_t kStackFrameSizeLimit = 3000;
#else
  uintptr_t kStackFrameSizeLimit = 1500;
#endif
  ASSERT_LE(outerStackPointer - innerStackPointer, kStackFrameSizeLimit);
}

// In release mode, we test the size of the interpreter's stack frame.
// "getSP" is installed as a native function and called from JS. The
// distance from its "inner" frame to the "outer" frame that invoked
// the interpreter is our approximate measure of the stack size used
// by the interpreter. We set a limit that will catch severe
// regressions, e.g., due to compiler quirks.
TEST_F(InterpreterTest, FrameSizeTest) {
  auto runtimeModule = RuntimeModule::createUninitialized(runtime, domain);

  /*
   get_global   reg0
   get_named    reg1, reg0, "getSP"
   call         reg0, reg1, 0
   ret          reg0
   */

  StringID getSPID = 1;

  const unsigned FRAME_SIZE = 16;
  BytecodeModuleGenerator BMG;
  auto BFG = BytecodeFunctionGenerator::create(BMG, FRAME_SIZE);

  BFG->emitGetGlobalObject(0);
  BFG->emitGetById(1, 0, 1, getSPID);
  BFG->emitCall(0, 1, 0);
  BFG->emitRet(0);
  BFG->setHighestReadCacheIndex(1);
  BFG->setHighestWriteCacheIndex(0);

  BFG->bytecodeGenerationComplete();
  auto codeBlock = createCodeBlock(runtimeModule, runtime, BFG.get());

  ASSERT_EQ(detail::mapStringMayAllocate(*runtimeModule, "getSP"), getSPID);

  auto getSPFn = runtime.makeHandle<NativeFunction>(
      *NativeFunction::createWithoutPrototype(
          runtime,
          nullptr,
          getSP,
          Predefined::getSymbolID(Predefined::emptyString),
          0));

  // Define the 'getSP' function.
  (void)JSObject::putNamed_RJS(
      runtime.getGlobal(),
      runtime,
      runtimeModule->getSymbolIDFromStringIDMayAllocate(getSPID),
      getSPFn);

  ScopedNativeCallFrame frame(
      runtime, 0, nullptr, false, HermesValue::encodeUndefinedValue());
  ASSERT_FALSE(frame.overflowed());

  testInterpreterStackSize(runtime, codeBlock);
}
#endif // NDEBUG

} // anonymous namespace
