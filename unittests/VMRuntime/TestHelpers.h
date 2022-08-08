/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_VMRUNTIME_TESTHELPERS_H
#define HERMES_UNITTESTS_VMRUNTIME_TESTHELPERS_H

#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/BCGen/HBC/BytecodeProviderFromSrc.h"
#include "hermes/Public/GCConfig.h"
#include "hermes/Public/JSOutOfMemoryError.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/Domain.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/RuntimeModule-inline.h"
#include "hermes/VM/StorageProvider.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringRefUtils.h"

#include "gtest/gtest.h"

namespace hermes {
namespace vm {

// Initial and max heap size constants
static constexpr uint32_t kInitHeapSmall = 1 << 8;
static constexpr uint32_t kMaxHeapSmall = 1 << 11;
static constexpr uint32_t kInitHeapSize = 1 << 16;
static constexpr uint32_t kMaxHeapSize = 1 << 19;
static constexpr uint32_t kInitHeapLarge = 1 << 20;
static constexpr uint32_t kMaxHeapLarge = 1 << 24;

static const GCConfig::Builder kTestGCConfigBaseBuilder =
    GCConfig::Builder().withSanitizeConfig(
        vm::GCSanitizeConfig::Builder().withSanitizeRate(0.0).build());

static const GCConfig kTestGCConfigSmall =
    GCConfig::Builder(kTestGCConfigBaseBuilder)
        .withInitHeapSize(kInitHeapSmall)
        .withMaxHeapSize(kMaxHeapSmall)
        .build();

static const GCConfig::Builder kTestGCConfigBuilder =
    GCConfig::Builder(kTestGCConfigBaseBuilder)
        .withInitHeapSize(kInitHeapSize)
        .withMaxHeapSize(kMaxHeapSize);

static const GCConfig kTestGCConfig =
    GCConfig::Builder(kTestGCConfigBuilder).build();

static const GCConfig kTestGCConfigLarge =
    GCConfig::Builder(kTestGCConfigBuilder)
        .withInitHeapSize(kInitHeapLarge)
        .withMaxHeapSize(kMaxHeapLarge)
        .build();

static const RuntimeConfig kTestRTConfigSmallHeap =
    RuntimeConfig::Builder().withGCConfig(kTestGCConfigSmall).build();

static const RuntimeConfig::Builder kTestRTConfigBuilder =
    RuntimeConfig::Builder().withGCConfig(kTestGCConfig);

static const RuntimeConfig kTestRTConfig =
    RuntimeConfig::Builder(kTestRTConfigBuilder).build();

static const RuntimeConfig kTestRTConfigLargeHeap =
    RuntimeConfig::Builder().withGCConfig(kTestGCConfigLarge).build();

template <typename T>
::testing::AssertionResult isException(
    Runtime &runtime,
    const CallResult<T> &res) {
  return isException(runtime, res.getStatus());
}

::testing::AssertionResult isException(
    Runtime &runtime,
    ExecutionStatus status);

/// A RuntimeTestFixture should be used by any test that requires a Runtime.
/// For different heap sizes, use the different subclasses.
class RuntimeTestFixtureBase : public ::testing::Test {
  std::shared_ptr<Runtime> rt;

 protected:
  // Convenience accessor that points to rt.
  Runtime &runtime;

  RuntimeConfig rtConfig;

  GCScope gcScope;

  Handle<Domain> domain;

  RuntimeTestFixtureBase(const RuntimeConfig &runtimeConfig)
      : rt(Runtime::create(runtimeConfig)),
        runtime(*rt),
        rtConfig(runtimeConfig),
        gcScope(runtime),
        domain(runtime.makeHandle(Domain::create(runtime))) {}

  /// Can't copy due to internal pointer.
  RuntimeTestFixtureBase(const RuntimeTestFixtureBase &) = delete;

  template <typename T>
  ::testing::AssertionResult isException(const CallResult<T> &res) {
    return isException(res.getStatus());
  }

  ::testing::AssertionResult isException(ExecutionStatus status) {
    return ::hermes::vm::isException(runtime, status);
  }

  std::shared_ptr<Runtime> newRuntime() {
    return Runtime::create(rtConfig);
  }
};

class RuntimeTestFixture : public RuntimeTestFixtureBase {
 public:
  RuntimeTestFixture() : RuntimeTestFixtureBase(kTestRTConfig) {}
  RuntimeTestFixture(experiments::VMExperimentFlags flags)
      : RuntimeTestFixtureBase(RuntimeConfig::Builder()
                                   .withGCConfig(kTestGCConfig)
                                   .withVMExperimentFlags(flags)
                                   .build()) {}
};

class SmallHeapRuntimeTestFixture : public RuntimeTestFixtureBase {
 public:
  SmallHeapRuntimeTestFixture()
      : RuntimeTestFixtureBase(kTestRTConfigSmallHeap) {}
};

class LargeHeapRuntimeTestFixture : public RuntimeTestFixtureBase {
 public:
  LargeHeapRuntimeTestFixture()
      : RuntimeTestFixtureBase(kTestRTConfigLargeHeap) {}
};

/// Configuration for the GC which fixes a size -- \p sz -- for the heap, and
/// does not permit any growth.  Intended only for testing purposes where we
/// don't expect or want the heap to grow.
///
/// \p builder is an optional parameter representing an initial builder to set
///     size parameters upon, providing an opportunity to set other parameters.
inline const GCConfig TestGCConfigFixedSize(
    gcheapsize_t sz,
    GCConfig::Builder builder = kTestGCConfigBuilder) {
  return builder.withInitHeapSize(sz).withMaxHeapSize(sz).build();
}

/// Assert that execution of `x' didn't throw.
#define ASSERT_RETURNED(x) ASSERT_EQ(ExecutionStatus::RETURNED, x)

/// Expect that 'x' is a string primitive with value 'str'
#define EXPECT_STRINGPRIM(str, x)                                           \
  do {                                                                      \
    GCScopeMarkerRAII marker{runtime};                                      \
    Handle<> xHandle{runtime, x};                                           \
    Handle<StringPrimitive> strHandle =                                     \
        StringPrimitive::createNoThrow(runtime, str);                       \
    EXPECT_TRUE(                                                            \
        isSameValue(strHandle.getHermesValue(), xHandle.getHermesValue())); \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned the expected bool.
#define EXPECT_CALLRESULT_BOOL_RAW(B, x) \
  do {                                   \
    auto res = x;                        \
    ASSERT_RETURNED(res.getStatus());    \
    EXPECT_##B(*res);                    \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned the expected bool.
#define EXPECT_CALLRESULT_BOOL(B, x)  \
  do {                                \
    auto res = x;                     \
    ASSERT_RETURNED(res.getStatus()); \
    EXPECT_##B((*res)->getBool());    \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned the expected
/// CallResult.
// Will replace "EXPECT_RETURN_STRING" after the entire refactor.
#define EXPECT_CALLRESULT_STRING(str, x) \
  do {                                   \
    auto res = x;                        \
    ASSERT_RETURNED(res.getStatus());    \
    EXPECT_STRINGPRIM(str, res->get());  \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned undefined.
#define EXPECT_CALLRESULT_UNDEFINED(x)  \
  do {                                  \
    auto res = x;                       \
    ASSERT_RETURNED(res.getStatus());   \
    EXPECT_TRUE((*res)->isUndefined()); \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned the expected double.
#define EXPECT_CALLRESULT_DOUBLE(d, x) \
  do {                                 \
    auto res = x;                      \
    ASSERT_RETURNED(res.getStatus());  \
    EXPECT_EQ(d, (*res)->getDouble()); \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned the expected double.
#define EXPECT_CALLRESULT_VALUE(v, x) \
  do {                                \
    auto res = x;                     \
    ASSERT_RETURNED(res.getStatus()); \
    EXPECT_EQ(v, res->get());         \
  } while (0)

/// Some tests expect out of memory.  This may either be fatal, or throw
/// exception; parameterize tests over this choice.
#ifdef HERMESVM_EXCEPTION_ON_OOM
#define EXPECT_OOM(exp)                     \
  {                                         \
    bool exThrown = false;                  \
    try {                                   \
      exp;                                  \
    } catch (const JSOutOfMemoryError &x) { \
      exThrown = true;                      \
    }                                       \
    EXPECT_TRUE(exThrown);                  \
  }
#else
#define EXPECT_OOM(exp) EXPECT_DEATH_IF_SUPPORTED({ exp; }, "OOM")
#endif

/// Get a named value from an object.
#define GET_VALUE(objHandle, predefinedId)                  \
  do {                                                      \
    propRes = JSObject::getNamed_RJS(                       \
        objHandle,                                          \
        runtime,                                            \
        Predefined::getSymbolID(Predefined::predefinedId)); \
    ASSERT_RETURNED(propRes.getStatus());                   \
  } while (0)

/// Get the global object.
#define GET_GLOBAL(predefinedId) GET_VALUE(runtime.getGlobal(), predefinedId)

inline HermesValue operator"" _hd(long double d) {
  return HermesValue::encodeDoubleValue(d);
}

/// A minimal Runtime for GC tests.
class DummyRuntime final : public HandleRootOwner,
                           public PointerBase,
                           private GCBase::GCCallbacks {
 private:
  GCStorage gcStorage_;

 public:
  std::vector<WeakRoot<GCCell> *> weakRoots{};

  /// Create a DummyRuntime with the default parameters.
  static std::shared_ptr<DummyRuntime> create(const GCConfig &gcConfig);

  /// Use a custom storage provider and/or a custom crash manager.
  /// \param provider A pointer to a StorageProvider. It *must* use
  ///   StorageProvider::defaultProvider eventually or the test will fail.
  /// \param crashMgr
  static std::shared_ptr<DummyRuntime> create(
      const GCConfig &gcConfig,
      std::shared_ptr<StorageProvider> provider,
      std::shared_ptr<CrashManager> crashMgr =
          std::make_shared<NopCrashManager>());

  /// Provide the correct storage provider based on build modes.
  /// All decorator StorageProviders must wrap the one returned from this
  /// function.
  static std::unique_ptr<StorageProvider> defaultProvider();

  ~DummyRuntime() override;

  template <
      typename T,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::No,
      class... Args>
  T *makeAFixed(Args &&...args) {
    return getHeap().makeAFixed<T, hasFinalizer, longLived>(
        std::forward<Args>(args)...);
  }

  template <
      typename T,
      HasFinalizer hasFinalizer = HasFinalizer::No,
      LongLived longLived = LongLived::No,
      class... Args>
  T *makeAVariable(uint32_t size, Args &&...args) {
    return getHeap().makeAVariable<T, hasFinalizer, longLived>(
        size, std::forward<Args>(args)...);
  }

  GC &getHeap() {
    return *gcStorage_.get();
  }

  void collect();

  void markRoots(RootAndSlotAcceptorWithNames &acceptor, bool) override;

  void markWeakRoots(WeakRootAcceptor &weakAcceptor, bool) override;

  void markRootsForCompleteMarking(
      RootAndSlotAcceptorWithNames &acceptor) override;

  unsigned int getSymbolsEnd() const override {
    return 0;
  }

  void unmarkSymbols() override {}

  void freeSymbols(const llvh::BitVector &) override {}

#ifdef HERMES_SLOW_DEBUG
  bool isSymbolLive(SymbolID) override {
    return true;
  }

  const void *getStringForSymbol(SymbolID) override {
    return nullptr;
  }
#endif

  void printRuntimeGCStats(JSONEmitter &) const override {}

  void visitIdentifiers(
      const std::function<void(SymbolID, const StringPrimitive *)> &) override {
  }

  std::string convertSymbolToUTF8(SymbolID) override;

  std::string getCallStackNoAlloc() override {
    return "<dummy runtime has no call stack>";
  }

  void onGCEvent(GCEventKind, const std::string &) override {}

  /// It's a unit test, it doesn't care about reporting how much memory it uses.
  size_t mallocSize() const override {
    return 0;
  }

  const inst::Inst *getCurrentIPSlow() const override {
    return nullptr;
  }

#ifdef HERMES_MEMORY_INSTRUMENTATION
  StackTracesTreeNode *getCurrentStackTracesTreeNode(
      const inst::Inst *ip) override {
    return nullptr;
  }

  StackTracesTree *getStackTracesTree() override {
    return nullptr;
  }
#endif

 private:
  DummyRuntime(
      const GCConfig &gcConfig,
      std::shared_ptr<StorageProvider> storageProvider,
      std::shared_ptr<CrashManager> crashMgr);
};

/// A DummyRuntimeTestFixtureBase should be used by any test that requires a
/// DummyRuntime. It takes a GCConfig, which can be
/// used to specify heap size using the constants i.e kInitHeapSize.
class DummyRuntimeTestFixtureBase : public ::testing::Test {
  std::shared_ptr<DummyRuntime> rt;

 protected:
  // Convenience accessor that points to rt.
  DummyRuntime &runtime;

  GCScope gcScope;

  DummyRuntimeTestFixtureBase(const GCConfig &gcConfig)
      : rt(DummyRuntime::create(gcConfig)), runtime(*rt), gcScope(runtime) {}

  /// Can't copy due to internal pointer.
  DummyRuntimeTestFixtureBase(const DummyRuntimeTestFixtureBase &) = delete;
};

// Provide HermesValue & wrappers comparison operators for convenience.

/// Compare two HermesValue for bit equality.
inline bool operator==(HermesValue a, HermesValue b) {
  return a.getRaw() == b.getRaw();
}

/// Helper function to create a CodeBlock that correspond to a single function
/// generated from \p BFG. The generated code block will be part of the \p
/// runtimeModule.
inline CodeBlock *createCodeBlock(
    RuntimeModule *runtimeModule,
    Runtime &,
    hbc::BytecodeFunctionGenerator *BFG) {
  std::unique_ptr<hbc::BytecodeModule> BM(new hbc::BytecodeModule(1));
  BM->setFunction(
      0,
      BFG->generateBytecodeFunction(
          Function::DefinitionKind::ES5Function,
          ValueKind::FunctionKind,
          true,
          0,
          0));
  runtimeModule->initializeWithoutCJSModulesMayAllocate(
      hbc::BCProviderFromSrc::createBCProviderFromSrc(std::move(BM)));
  return runtimeModule->getCodeBlockMayAllocate(0);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_UNITTESTS_VMRUNTIME_TESTHELPERS_H
