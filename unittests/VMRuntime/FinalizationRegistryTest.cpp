/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "VMRuntimeTestHelpers.h"
#include "hermes/VM/JSFinalizationRegistry.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using FinalizationRegistryTest = RuntimeTestFixture;

/// No-op cleanup callback.
static CallResult<HermesValue> noopCallback(void *, Runtime &) {
  return HermesValue::encodeUndefinedValue();
}

CallResult<HermesValue> eval(Runtime &runtime, llvh::StringRef code) {
  hermes::hbc::CompileFlags flags;
  return runtime.run(code, "", flags);
}

TEST_F(FinalizationRegistryTest, RegisterAndUnregisterTest) {
  WeakSmallHermesValue heldValue;
  WeakSmallHermesValue fr;
  runtime.addCustomWeakRootsFunction([&](GC *gc, WeakRootAcceptor &acceptor) {
    acceptor.acceptWeak(fr);
    acceptor.acceptWeak(heldValue);
  });

  struct : Locals {
    PinnedValue<JSFinalizationRegistry> fr;
    PinnedValue<Callable> callable;
    PinnedValue<JSObject> heldValue;
    PinnedValue<> target;
    PinnedValue<JSObject> unregisterToken;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  lv.callable = *NativeFunction::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      Runtime::makeNullHandle<Environment>(),
      nullptr,
      noopCallback,
      Predefined::getSymbolID(Predefined::emptyString),
      0,
      Runtime::makeNullHandle<JSObject>());

  auto frRes = JSFinalizationRegistry::create(
      runtime, runtime.arrayPrototype, lv.callable);
  lv.fr = std::move(frRes);
  fr.setObject(runtime, *lv.fr);

  lv.heldValue = JSObject::create(runtime);
  heldValue.set(SmallHermesValue::encodeObjectValue(*lv.heldValue, runtime));
  lv.target = JSObject::create(runtime);
  lv.unregisterToken = JSObject::create(runtime);
  JSFinalizationRegistry::registerCell(
      lv.fr, runtime, lv.target, lv.heldValue, lv.unregisterToken);

  lv.heldValue = nullptr;
  // Trigger a GC
  runtime.collect("test");
  ASSERT_FALSE(heldValue.isInvalid());
  ASSERT_FALSE(fr.isInvalid());

  lv.target = JSObject::create(runtime);
  lv.heldValue = lv.fr;
  JSFinalizationRegistry::registerCell(
      lv.fr, runtime, lv.target, lv.heldValue, runtime.getUndefinedValue());
  bool success = lv.fr->unregisterCells(runtime, lv.unregisterToken);
  // Should unregister the first record.
  ASSERT_TRUE(success);
  // Now GC should invalidate heldValue of first record.
  runtime.collect("test");
  // heldValue of the unregistered record should be invalid now.
  ASSERT_TRUE(heldValue.isInvalid());

  // Test with Symbol Target
  lv.heldValue = JSObject::create(runtime);
  heldValue.set(SmallHermesValue::encodeObjectValue(*lv.heldValue, runtime));
  auto desc = StringPrimitive::createNoThrow(runtime, "test");
  auto sym = runtime.getIdentifierTable().createNotUniquedSymbol(runtime, desc);
  lv.target = HermesValue::encodeSymbolValue(*sym);
  lv.unregisterToken = JSObject::create(runtime);
  JSFinalizationRegistry::registerCell(
      lv.fr, runtime, lv.target, lv.heldValue, lv.unregisterToken);
  lv.heldValue = nullptr;
  runtime.collect("test");
  ASSERT_FALSE(heldValue.isInvalid());
  bool successSym = lv.fr->unregisterCells(runtime, lv.unregisterToken);
  ASSERT_TRUE(successSym);

  runtime.collect("test");
  ASSERT_TRUE(heldValue.isInvalid());

  lv.target = Runtime::getUndefinedValue();
  lv.fr = nullptr;
  lv.heldValue = nullptr;
  // Now fr is only referenced by heldValue of second record, which should be
  // already dead.
  runtime.collect("test");
  // fr should be freed.
  ASSERT_TRUE(fr.isInvalid());
}

TEST_F(FinalizationRegistryTest, CleanupTest) {
  struct : Locals {
    PinnedValue<JSFinalizationRegistry> fr;
    PinnedValue<Callable> callable;
    PinnedValue<JSObject> heldValue;
    PinnedValue<> target;
    PinnedValue<JSObject> unregisterToken;
    PinnedValue<> counter;
  } lv;
  LocalsRAII lraii{runtime, &lv};

  // Let the callback trigger a GC, to ensure that caller does consider it.
  auto callableRes = eval(runtime, R"#(
    globalThis.counter = 0;
    function cleanup() { gc(); globalThis.counter += 1; }
    cleanup
  )#");
  lv.callable.castAndSetHermesValue<Callable>(*callableRes);

  auto counterSym = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createASCIIRef("counter"));
  auto getCounter = [this, counterSym]() {
    auto result =
        JSObject::getNamed_RJS(runtime.getGlobal(), runtime, *counterSym);
    return (*result)->getNumberAs<int>();
  };

  auto frRes = JSFinalizationRegistry::create(
      runtime, runtime.arrayPrototype, lv.callable);
  lv.fr = std::move(frRes);
  lv.heldValue = JSObject::create(runtime);

  for (size_t i = 0; i < 3; ++i) {
    lv.target = JSObject::create(runtime);
    JSFinalizationRegistry::registerCell(
        lv.fr, runtime, lv.target, lv.heldValue, runtime.getUndefinedValue());
  }

  runtime.collect("test");
  runtime.cleanUpFinalizationCallbacks();
  // The first 2 records (objects) will be cleaned up.
  ASSERT_EQ(getCounter(), 2);

  for (size_t i = 0; i < 3; ++i) {
    auto desc = StringPrimitive::createNoThrow(runtime, "test");
    auto sym =
        runtime.getIdentifierTable().createNotUniquedSymbol(runtime, desc);
    lv.target = HermesValue::encodeSymbolValue(*sym);
    JSFinalizationRegistry::registerCell(
        lv.fr, runtime, lv.target, lv.heldValue, runtime.getUndefinedValue());
  }

  runtime.collect("test");
  runtime.cleanUpFinalizationCallbacks();
  // The first 2 symbol targets will be cleaned up.
  ASSERT_EQ(getCounter(), 5);

  lv.target = JSObject::create(runtime);
  lv.unregisterToken = JSObject::create(runtime);
  JSFinalizationRegistry::registerCell(
      lv.fr, runtime, lv.target, lv.heldValue, lv.unregisterToken);
  ASSERT_TRUE(lv.fr->unregisterCells(runtime, lv.unregisterToken));

  auto desc = StringPrimitive::createNoThrow(runtime, "test");
  auto sym = runtime.getIdentifierTable().createNotUniquedSymbol(runtime, desc);
  lv.target = HermesValue::encodeSymbolValue(*sym);
  JSFinalizationRegistry::registerCell(
      lv.fr, runtime, lv.target, lv.heldValue, lv.unregisterToken);
  ASSERT_TRUE(lv.fr->unregisterCells(runtime, lv.unregisterToken));

  runtime.collect("test");
  runtime.cleanUpFinalizationCallbacks();
  // The last target is unregistered. All previous target are cleaned up.
  ASSERT_EQ(getCounter(), 6);
}

} // namespace
