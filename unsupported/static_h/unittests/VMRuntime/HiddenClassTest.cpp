/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/HiddenClass.h"

#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Runtime.h"

#include "TestHelpers.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using HiddenClassTest = LargeHeapRuntimeTestFixture;

TEST_F(HiddenClassTest, SmokeTest) {
  GCScope gcScope{runtime, "HiddenClassTest.SmokeTest", 48};

  runtime.collect("test");

  auto aHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"a"));
  auto bHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"b"));
  auto cHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"c"));
  auto dHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"d"));

  // We will simulate and verify the following property operations, starting
  // from the same root.
  /// x.a, x.b
  /// y.a, y.b
  /// x.c
  /// y.d
  /// read-only x.b
  /// z.a, z.b, z.c
  /// read-only z.b
  /// all-read-only y twice

  MutableHandle<HiddenClass> x{runtime};
  MutableHandle<HiddenClass> y{runtime};
  MutableHandle<HiddenClass> z{runtime};

  auto rootHnd = runtime.makeHandle<HiddenClass>(
      runtime.ignoreAllocationFailure(HiddenClass::createRoot(runtime)));

  ASSERT_EQ(0u, rootHnd->getNumProperties());
  ASSERT_FALSE(rootHnd->isDictionary());
  ASSERT_TRUE(rootHnd->isKnownLeaf());

  // x = {}
  x = *rootHnd;
  {
    // x.a
    auto addRes = HiddenClass::addProperty(
        x, runtime, *aHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(0u, addRes->second);
    ASSERT_NE(*rootHnd, *addRes->first);
    x = *addRes->first;
  }
  {
    // x.b
    auto addRes = HiddenClass::addProperty(
        x, runtime, *bHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(1u, addRes->second);
    ASSERT_NE(*x, *addRes->first);
    x = *addRes->first;
  }
  // y = {}
  y = *rootHnd;
  {
    // y.a
    auto addRes = HiddenClass::addProperty(
        y, runtime, *aHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(0u, addRes->second);
    y = *addRes->first;
  }
  {
    // y.b
    auto addRes = HiddenClass::addProperty(
        y, runtime, *bHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(1u, addRes->second);
    y = *addRes->first;
    ASSERT_EQ(*x, *y);
  }
  {
    // x.c
    auto addRes = HiddenClass::addProperty(
        x, runtime, *cHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(2u, addRes->second);
    ASSERT_NE(*x, *addRes->first);
    x = *addRes->first;
    ASSERT_EQ(3u, x->getNumProperties());
    ASSERT_FALSE(x->isDictionary());
    ASSERT_TRUE(x->isKnownLeaf());
  }
  {
    // y.d
    auto addRes = HiddenClass::addProperty(
        y, runtime, *dHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(2u, addRes->second);
    y = *addRes->first;
    ASSERT_NE(*x, *y);
    ASSERT_EQ(3u, y->getNumProperties());
    ASSERT_FALSE(y->isDictionary());
    ASSERT_TRUE(y->isKnownLeaf());
  }

  // Find all properties in x.
  NamedPropertyDescriptor desc;
  {
    auto found = HiddenClass::findProperty(
        x, runtime, *aHnd, PropertyFlags::invalid(), desc);
    ASSERT_TRUE(found);
    ASSERT_EQ(0u, desc.slot);
    found = HiddenClass::findProperty(
        x, runtime, *bHnd, PropertyFlags::invalid(), desc);
    ASSERT_TRUE(found);
    ASSERT_EQ(1u, desc.slot);
    found = HiddenClass::findProperty(
        x, runtime, *cHnd, PropertyFlags::invalid(), desc);
    ASSERT_TRUE(found);
    ASSERT_EQ(2u, desc.slot);
    found = HiddenClass::findProperty(
        x, runtime, *dHnd, PropertyFlags::invalid(), desc);
    ASSERT_FALSE(found);
  }

  {
    // Read-only x.b
    auto found = HiddenClass::findProperty(
        x, runtime, *bHnd, PropertyFlags::invalid(), desc);
    ASSERT_TRUE(found);
    ASSERT_EQ(1u, desc.slot);
    ASSERT_TRUE(desc.flags.writable);

    desc.flags.writable = false;
    auto newClz = HiddenClass::updateProperty(x, runtime, *found, desc.flags);
    ASSERT_NE(*x, *newClz);
    ASSERT_EQ(x->getNumProperties(), newClz->getNumProperties());
    x = *newClz;

    found = HiddenClass::findProperty(
        x, runtime, *bHnd, PropertyFlags::invalid(), desc);
    ASSERT_TRUE(found);
    ASSERT_EQ(1u, desc.slot);
    ASSERT_FALSE(desc.flags.writable);
  }

  // z = {}
  z = *rootHnd;
  {
    // z.a
    auto addRes = HiddenClass::addProperty(
        z, runtime, *aHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(0u, addRes->second);
    z = *addRes->first;
  }
  {
    // z.b
    auto addRes = HiddenClass::addProperty(
        z, runtime, *bHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(1u, addRes->second);
    z = *addRes->first;
  }
  {
    // z.c
    auto addRes = HiddenClass::addProperty(
        z, runtime, *cHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(2u, addRes->second);
    z = *addRes->first;
  }

  {
    // Read-only z.b
    auto found = HiddenClass::findProperty(
        z, runtime, *bHnd, PropertyFlags::invalid(), desc);
    ASSERT_TRUE(found);
    ASSERT_EQ(1u, desc.slot);
    ASSERT_TRUE(desc.flags.writable);

    desc.flags.writable = false;
    auto newClz = HiddenClass::updateProperty(z, runtime, *found, desc.flags);
    ASSERT_NE(*z, *newClz);
    ASSERT_EQ(z->getNumProperties(), newClz->getNumProperties());
    z = *newClz;

    found = HiddenClass::findProperty(
        z, runtime, *bHnd, PropertyFlags::invalid(), desc);
    ASSERT_TRUE(found);
    ASSERT_EQ(1u, desc.slot);
    ASSERT_FALSE(desc.flags.writable);

    ASSERT_EQ(*x, *z);
  }

  auto y1 = HiddenClass::makeAllReadOnly(y, runtime);
  auto y2 = HiddenClass::makeAllReadOnly(y, runtime);
  ASSERT_EQ(*y1, *y2);
  auto y3 = HiddenClass::makeAllReadOnly(y1, runtime);
  ASSERT_EQ(*y1, *y3);

  // Turn x into a dictionary by erasing x.a

  {
    auto found = HiddenClass::findProperty(
        x, runtime, *aHnd, PropertyFlags::invalid(), desc);
    ASSERT_TRUE(found);
    ASSERT_EQ(0u, desc.slot);

    auto x1 = HiddenClass::deleteProperty(x, runtime, *found);
    ASSERT_NE(*x, *x1);
    ASSERT_FALSE(x->isDictionary());
    ASSERT_TRUE(x1->isDictionary());
    ASSERT_EQ(2u, x1->getNumProperties());

    found = HiddenClass::findProperty(
        x1, runtime, *aHnd, PropertyFlags::invalid(), desc);
    ASSERT_FALSE(found);

    x = *x1;
  }

  {
    // x.a (again)
    auto addRes = HiddenClass::addProperty(
        x, runtime, *aHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(0u, addRes->second);
    ASSERT_EQ(*x, *addRes->first);
    ASSERT_EQ(3u, x->getNumProperties());
  }
}

TEST_F(HiddenClassTest, UpdatePropertyFlagsWithoutTransitionsTest) {
  GCScope gcScope{
      runtime, "HiddenClassTest.UpdatePropertyFlagsWithoutTransitionsTest", 48};

  runtime.collect("test");

  auto aHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"a"));
  auto bHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"b"));
  auto cHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"c"));
  auto dHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"d"));

  // Add y.a, y.b, y.c
  MutableHandle<HiddenClass> y{
      runtime,
      vmcast<HiddenClass>(
          runtime.ignoreAllocationFailure(HiddenClass::createRoot(runtime)))};
  {
    // y.a
    auto addRes = HiddenClass::addProperty(
        y, runtime, *aHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(0u, addRes->second);
    y = *addRes->first;
  }
  {
    // y.b
    auto addRes = HiddenClass::addProperty(
        y, runtime, *bHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(1u, addRes->second);
    y = *addRes->first;
  }
  {
    // y.c
    auto addRes = HiddenClass::addProperty(
        y, runtime, *cHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(2u, addRes->second);
    y = *addRes->first;
  }

  NamedPropertyDescriptor desc;

  PropertyFlags clearFlags;
  clearFlags.writable = 1;
  clearFlags.configurable = 1;
  PropertyFlags setFlags;

  ASSERT_FALSE(y->isDictionary());
  // y is not a dictionary so we will get a new hidden class.
  auto yClone = HiddenClass::updatePropertyFlagsWithoutTransitions(
      y, runtime, clearFlags, setFlags, llvh::None);
  ASSERT_NE(*y, *yClone);
  ASSERT_EQ(y->getNumProperties(), yClone->getNumProperties());
  ASSERT_TRUE(yClone->isDictionary());
  // Check each property
  auto found = HiddenClass::findProperty(
      yClone, runtime, *aHnd, PropertyFlags::invalid(), desc);
  ASSERT_TRUE(found);
  ASSERT_EQ(0u, desc.slot);
  ASSERT_FALSE(desc.flags.writable);
  ASSERT_FALSE(desc.flags.configurable);

  found = HiddenClass::findProperty(
      yClone, runtime, *bHnd, PropertyFlags::invalid(), desc);
  ASSERT_TRUE(found);
  ASSERT_EQ(1u, desc.slot);
  ASSERT_FALSE(desc.flags.writable);
  ASSERT_FALSE(desc.flags.configurable);

  found = HiddenClass::findProperty(
      yClone, runtime, *cHnd, PropertyFlags::invalid(), desc);
  ASSERT_TRUE(found);
  ASSERT_EQ(2u, desc.slot);
  ASSERT_FALSE(desc.flags.writable);
  ASSERT_FALSE(desc.flags.configurable);

  // Turn y into a dictionary y3 by deleting y.a
  found = HiddenClass::findProperty(
      y, runtime, *aHnd, PropertyFlags::invalid(), desc);
  ASSERT_TRUE(found);
  ASSERT_EQ(0u, desc.slot);

  auto y3 = HiddenClass::deleteProperty(y, runtime, *found);
  ASSERT_NE(*y, *y3);
  ASSERT_TRUE(y3->isDictionary());
  ASSERT_EQ(2u, y3->getNumProperties());
  // We should not create a new hidden class in this case.
  auto y4 = HiddenClass::updatePropertyFlagsWithoutTransitions(
      y3, runtime, clearFlags, setFlags, llvh::None);
  ASSERT_EQ(*y4, *y3);

  // Only freeze y.a and y.c
  std::vector<SymbolID> propsToFreeze;
  propsToFreeze.push_back(*aHnd);
  propsToFreeze.push_back(*cHnd);
  propsToFreeze.push_back(*dHnd); // This is not in the map yet.
  // Freeze while only create a singleton hidden class.
  auto partlyFrozenSingleton =
      HiddenClass::updatePropertyFlagsWithoutTransitions(
          y,
          runtime,
          clearFlags,
          setFlags,
          llvh::ArrayRef<SymbolID>(propsToFreeze));

  ASSERT_NE(*y, *partlyFrozenSingleton);
  ASSERT_EQ(y->getNumProperties(), partlyFrozenSingleton->getNumProperties());
  ASSERT_TRUE(partlyFrozenSingleton->isDictionary());
  // Check each property
  found = HiddenClass::findProperty(
      partlyFrozenSingleton, runtime, *aHnd, PropertyFlags::invalid(), desc);
  ASSERT_TRUE(found);
  ASSERT_EQ(0u, desc.slot);
  ASSERT_FALSE(desc.flags.writable);
  ASSERT_FALSE(desc.flags.configurable);

  found = HiddenClass::findProperty(
      partlyFrozenSingleton, runtime, *bHnd, PropertyFlags::invalid(), desc);
  ASSERT_TRUE(found);
  ASSERT_EQ(1u, desc.slot);
  ASSERT_TRUE(desc.flags.writable);
  ASSERT_TRUE(desc.flags.configurable);

  found = HiddenClass::findProperty(
      partlyFrozenSingleton, runtime, *cHnd, PropertyFlags::invalid(), desc);
  ASSERT_TRUE(found);
  ASSERT_EQ(2u, desc.slot);
  ASSERT_FALSE(desc.flags.writable);
  ASSERT_FALSE(desc.flags.configurable);

  // We can still add another property to it.
  auto addRes = HiddenClass::addProperty(
      partlyFrozenSingleton,
      runtime,
      *dHnd,
      PropertyFlags::defaultNewNamedPropertyFlags());
  ASSERT_RETURNED(addRes);
  ASSERT_EQ(3u, addRes->second);
  ASSERT_EQ(*addRes->first, *partlyFrozenSingleton);
  ASSERT_EQ(addRes->first->getNumProperties(), 4);
}

TEST_F(HiddenClassTest, ForEachProperty) {
  MutableHandle<HiddenClass> clazz{
      runtime,
      vmcast<HiddenClass>(
          runtime.ignoreAllocationFailure(HiddenClass::createRoot(runtime)))};

  auto aHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"a"));
  auto bHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"b"));

  {
    // clazz.a
    auto addRes = HiddenClass::addProperty(
        clazz, runtime, *aHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(0u, addRes->second);
    clazz = *addRes->first;
  }
  {
    // clazz.b
    auto addRes = HiddenClass::addProperty(
        clazz, runtime, *bHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    ASSERT_EQ(1u, addRes->second);
    clazz = *addRes->first;
  }

  std::vector<std::pair<SymbolID, NamedPropertyDescriptor>> expectedProperties{
      {aHnd.get(),
       NamedPropertyDescriptor{
           PropertyFlags::defaultNewNamedPropertyFlags(), 0}},
      {bHnd.get(),
       NamedPropertyDescriptor{
           PropertyFlags::defaultNewNamedPropertyFlags(), 1}}};

  std::vector<std::pair<SymbolID, NamedPropertyDescriptor>> properties;
  HiddenClass::forEachProperty(
      clazz, runtime, [&properties](SymbolID id, NamedPropertyDescriptor desc) {
        properties.emplace_back(id, desc);
      });
  EXPECT_EQ(expectedProperties, properties);

  std::vector<std::pair<SymbolID, NamedPropertyDescriptor>> propertiesNoAlloc;
  HiddenClass::forEachPropertyNoAlloc(
      clazz.get(),
      runtime,
      [&propertiesNoAlloc](SymbolID id, NamedPropertyDescriptor desc) {
        propertiesNoAlloc.emplace_back(id, desc);
      });
  EXPECT_EQ(expectedProperties, propertiesNoAlloc);
}

TEST_F(HiddenClassTest, ReservedSlots) {
  auto aHnd = *runtime.getIdentifierTable().getSymbolHandle(
      runtime, createUTF16Ref(u"a"));
  for (unsigned i = 0; i <= InternalProperty::NumAnonymousInternalProperties;
       ++i) {
    Handle<HiddenClass> clazz =
        runtime.getHiddenClassForPrototype(*runtime.getGlobal(), i);
    EXPECT_FALSE(clazz->isDictionary());
    auto addRes = HiddenClass::addProperty(
        clazz, runtime, *aHnd, PropertyFlags::defaultNewNamedPropertyFlags());
    ASSERT_RETURNED(addRes);
    EXPECT_EQ(i, addRes->second);
  }
}

} // namespace
