/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_UNITTESTS_VMRUNTIME_TESTHELPERS_H
#define HERMES_UNITTESTS_VMRUNTIME_TESTHELPERS_H

#include "hermes/BCGen/HBC/BytecodeGenerator.h"
#include "hermes/Public/GCConfig.h"
#include "hermes/Public/RuntimeConfig.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/CodeBlock.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StorageProvider.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringRefUtils.h"

#include "gtest/gtest.h"

namespace hermes {
namespace vm {

static const GCConfig::Builder kTestGCConfigBuilder =
    GCConfig::Builder().withSanitizeRate(0.0).withShouldRandomizeAllocSpace(
        false);

static const GCConfig kTestGCConfigSmall =
    GCConfig::Builder(kTestGCConfigBuilder)
        .withInitHeapSize(1 << 8)
        .withMaxHeapSize(1 << 11)
        .build();

static const GCConfig kTestGCConfig = GCConfig::Builder(kTestGCConfigBuilder)
                                          .withInitHeapSize(1 << 16)
                                          .withMaxHeapSize(1 << 19)
                                          .build();

static const GCConfig kTestGCConfigLarge =
    GCConfig::Builder(kTestGCConfigBuilder)
        .withInitHeapSize(1 << 20)
        .withMaxHeapSize(1 << 23)
        .build();

static const RuntimeConfig kTestRTConfigSmallHeap =
    RuntimeConfig::Builder().withGCConfig(kTestGCConfigSmall).build();

static const RuntimeConfig kTestRTConfig =
    RuntimeConfig::Builder().withGCConfig(kTestGCConfig).build();

static const RuntimeConfig kTestRTConfigLargeHeap =
    RuntimeConfig::Builder().withGCConfig(kTestGCConfigLarge).build();

/// A RuntimeTestFixture should be used by any test that requires a Runtime.
/// For different heap sizes, use the different subclasses.
class RuntimeTestFixtureBase : public ::testing::Test {
  std::shared_ptr<Runtime> rt;

 protected:
  // Convenience accessor that points to rt.
  Runtime *runtime;

  GCScope gcScope;

  RuntimeTestFixtureBase(const RuntimeConfig &runtimeConfig)
      : rt(Runtime::create(runtimeConfig)),
        runtime(rt.get()),
        gcScope(runtime) {}

  /// Can't copy due to internal pointer.
  RuntimeTestFixtureBase(const RuntimeTestFixtureBase &) = delete;
};

class RuntimeTestFixture : public RuntimeTestFixtureBase {
 public:
  RuntimeTestFixture() : RuntimeTestFixtureBase(kTestRTConfig) {}
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
#define EXPECT_STRINGPRIM(str, x) \
  EXPECT_TRUE(isSameValue(        \
      StringPrimitive::createNoThrow(runtime, str).getHermesValue(), x));

/// Assert that execution of 'x' didn't throw and returned the expected bool.
#define EXPECT_CALLRESULT_BOOL(B, x)  \
  do {                                \
    auto res = x;                     \
    ASSERT_RETURNED(res.getStatus()); \
    EXPECT_##B(res->getBool());       \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned the expected
/// CallResult.
// Will replace "EXPECT_RETURN_STRING" after the entire refactor.
#define EXPECT_CALLRESULT_STRING(str, x)    \
  do {                                      \
    auto res = x;                           \
    ASSERT_RETURNED(res.getStatus());       \
    EXPECT_STRINGPRIM(str, res.getValue()); \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned undefined.
#define EXPECT_CALLRESULT_UNDEFINED(x) \
  do {                                 \
    auto res = x;                      \
    ASSERT_RETURNED(res.getStatus());  \
    EXPECT_TRUE(res->isUndefined());   \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned the expected double.
#define EXPECT_CALLRESULT_DOUBLE(d, x) \
  do {                                 \
    auto res = x;                      \
    ASSERT_RETURNED(res.getStatus());  \
    EXPECT_EQ(d, res->getDouble());    \
  } while (0)

/// Assert that execution of 'x' didn't throw and returned the expected double.
#define EXPECT_CALLRESULT_VALUE(v, x) \
  do {                                \
    auto res = x;                     \
    ASSERT_RETURNED(res.getStatus()); \
    EXPECT_EQ(v, res.getValue());     \
  } while (0)

/// Get a named value from an object.
#define GET_VALUE(objHandle, predefinedId)                         \
  do {                                                             \
    propRes = JSObject::getNamed(                                  \
        objHandle,                                                 \
        runtime,                                                   \
        runtime->getPredefinedSymbolID(Predefined::predefinedId)); \
    ASSERT_RETURNED(propRes.getStatus());                          \
  } while (0)

/// Get the global object.
#define GET_GLOBAL(predefinedId) GET_VALUE(runtime->getGlobal(), predefinedId)

inline HermesValue operator"" _hd(long double d) {
  return HermesValue::encodeDoubleValue(d);
}

/// A MetadataTable that has a public constructor for tests to use.
class MetadataTableForTests final : public MetadataTable {
 public:
  // The constructor is explicit to avoid incompatibilities between compilers.
  template <int length>
  explicit constexpr MetadataTableForTests(const Metadata (&table)[length])
      : MetadataTable(table) {}
};

/// A Runtime that can take a custom VTableMap and Metadata table.
struct DummyRuntime final : public HandleRootOwner,
                            private GCBase::GCCallbacks {
  GC gc;
  std::vector<GCCell **> pointerRoots{};
  std::vector<HermesValue *> valueRoots{};
  std::vector<void **> weakRoots{};
  std::function<void(GC *, SlotAcceptor &)> markExtra{};

  /// Create a DummyRuntime with the default parameters.
  static std::shared_ptr<DummyRuntime> create(
      MetadataTableForTests metaTable,
      const GCConfig &gcConfig);

  /// Use a custom storage provider.
  /// \param provider A pointer to a StorageProvider. It *must* use
  ///   StorageProvider::defaultProvider eventually or the test will fail.
  static std::shared_ptr<DummyRuntime> create(
      MetadataTableForTests metaTable,
      const GCConfig &gcConfig,
      std::unique_ptr<StorageProvider> provider);

  ~DummyRuntime() override {
    gc.finalizeAll();
  }

  template <bool fixedSize = true>
  void *alloc(uint32_t sz) {
    return gc.alloc<fixedSize>(sz);
  }
  template <HasFinalizer hasFinalizer = HasFinalizer::No>
  void *allocLongLived(uint32_t sz) {
    return gc.allocLongLived<hasFinalizer>(sz);
  }
  template <bool fixedSize = true>
  void *allocWithFinalizer(uint32_t sz) {
    return gc.alloc<fixedSize, HasFinalizer::Yes>(sz);
  }

  GC &getHeap() {
    return gc;
  }

  void markRoots(GC *gc, SlotAcceptorWithNames &acceptor, bool) override {
    markGCScopes(acceptor);
    for (GCCell **pp : pointerRoots)
      acceptor.acceptPtr(*pp);
    for (HermesValue *pp : valueRoots)
      acceptor.accept(*pp);

    if (markExtra)
      markExtra(gc, acceptor);
  }

  void markWeakRoots(GCBase *gc, SlotAcceptorWithNames &acceptor) override {
    for (void **ptr : weakRoots) {
      acceptor.accept(*ptr);
    }
  }

  unsigned int getSymbolsEnd() const override {
    return 0;
  }

  void freeSymbols(const std::vector<bool> &) override {}

  void printRuntimeGCStats(llvm::raw_ostream &) const override {}

  void visitIdentifiers(
      const std::function<void(UTF16Ref, uint32_t)> &acceptor) override {
    acceptor(createUTF16Ref(u"DummyIdTableEntry0"), 0);
    acceptor(createUTF16Ref(u"DummyIdTableEntry1"), 1);
  }

  /// It's a unit test, it doesn't care about reporting how much memory it uses.
  size_t mallocSize() const override {
    return 0;
  }

 private:
  DummyRuntime(
      MetadataTableForTests metaTable,
      const GCConfig &gcConfig,
      std::shared_ptr<StorageProvider> storageProvider)
      : gc(metaTable, this, gcConfig, storageProvider.get()) {}
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
    hbc::BytecodeFunctionGenerator *BFG) {
  std::unique_ptr<hbc::BytecodeModule> BM(new hbc::BytecodeModule(1));
  BM->setFunction(
      0,
      BFG->generateBytecodeFunction(
          Function::DefinitionKind::ES5Function, true, 0, 0));
  runtimeModule->initialize(
      hbc::BCProviderFromSrc::createBCProviderFromSrc(std::move(BM)));

  return CodeBlock::createCodeBlock(
      runtimeModule,
      runtimeModule->getBytecode()->getFunctionHeader(0),
      runtimeModule->getBytecode()->getBytecode(0),
      0);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_UNITTESTS_VMRUNTIME_TESTHELPERS_H
