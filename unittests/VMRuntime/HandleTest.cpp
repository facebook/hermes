/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#include "hermes/BCGen/HBC/Bytecode.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"

#include "gtest/gtest.h"

using namespace hermes::vm;

namespace {

using HandleTest = RuntimeTestFixture;
using HandleDeathTest = RuntimeTestFixture;

TEST_F(HandleTest, AddAndRemoveTest) {
  auto str1 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"str1"));
  auto str2 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"str2"));
  auto str3 = StringPrimitive::createNoThrow(runtime, createUTF16Ref(u"str3"));

  {
    Handle<StringPrimitive> h1(runtime, str1.get());
    {
      GCScope scope(runtime);
      Handle<StringPrimitive> h2(runtime, str2.get());
      Handle<StringPrimitive> h3(runtime, str3.get());

      ASSERT_EQ(str1, h1);
      ASSERT_EQ(str2, h2);
      ASSERT_EQ(str3, h3);
    }
    ASSERT_EQ(str1, h1);

    {
      GCScope scope(runtime);
      // Check operator=.
      Handle<StringPrimitive> h3 = h1;
      MutableHandle<StringPrimitive> h4(runtime, h1.get());
      Handle<StringPrimitive> h5 = h4;

      ASSERT_EQ(str1, h3);
      ASSERT_EQ(str1, h4);
      ASSERT_EQ(str1, h5);
      h4 = h1.get();

      ASSERT_EQ(str1, h4);
    }
    {
      GCScope scope(runtime);
      // Check copy constructor.
      Handle<StringPrimitive> h3(h1);
      Handle<StringPrimitive> h4(h1);

      ASSERT_EQ(str1, h3);
      ASSERT_EQ(str1, h4);
    }

    {
      GCScope scope(runtime);
      // Check mov constructor.
      Handle<StringPrimitive> h5(Handle<StringPrimitive>(runtime, str2.get()));
      ASSERT_EQ(str2, h5);
    }
  }
}

TEST_F(HandleTest, ValueAddAndRemoveTest) {
  auto str1 = Handle<>(runtime, HermesValue::encodeNumberValue(1));
  auto str2 = Handle<>(runtime, HermesValue::encodeNumberValue(2));
  auto str3 = Handle<>(runtime, HermesValue::encodeNumberValue(3));

  {
    MutableHandle<> h1(runtime, str1.get());
    {
      MutableHandle<> h2(runtime, str2.get());
      Handle<> h3(runtime, str3.get());

      ASSERT_EQ(str1, h1);
      ASSERT_EQ(str2, h2);
      ASSERT_EQ(str3, h3);

      h2.set(h3.get());
      ASSERT_EQ(str3, h2);
    }
    // h2 is out of scope, so it should be gone.

    {
      // Check operator=.
      Handle<> h3 = h1;
      MutableHandle<> h4(runtime, h1.get());
      Handle<> h5 = h4;

      ASSERT_EQ(str1, h3);
      ASSERT_EQ(str1, h4);
      ASSERT_EQ(str1, h5);
      h4 = h1.get();

      ASSERT_EQ(str1, h4);
    }
    {
      // Check copy constructor.
      Handle<> h3(h1);
      Handle<> h4(h1);

      ASSERT_EQ(str1, h3);
      ASSERT_EQ(str1, h4);
    }

    {
      // Check mov constructor.
      MutableHandle<> h5(runtime, h1.get());
      MutableHandle<> h6(std::move(h5));
      ASSERT_EQ(str1, h6);
    }
  }
}

TEST_F(HandleTest, MarkTest) {
  GCScope gcScope{runtime};
  Handle<> v1(runtime, HermesValue::encodeNumberValue(1));
  (void)v1;
  Handle<> v2(runtime, HermesValue::encodeNumberValue(2));
  (void)v2;
  ASSERT_EQ(2u, gcScope.getHandleCountDbg());

  auto marker = gcScope.createMarker();

  Handle<> v3(runtime, HermesValue::encodeNumberValue(3));
  (void)v3;
  ASSERT_EQ(3u, gcScope.getHandleCountDbg());

  gcScope.flushToMarker(marker);
  ASSERT_EQ(2u, gcScope.getHandleCountDbg());

  Handle<> v4(runtime, HermesValue::encodeNumberValue(4));
  (void)v4;
  ASSERT_EQ(3u, gcScope.getHandleCountDbg());

  gcScope.flushToMarker(marker);
  ASSERT_EQ(2u, gcScope.getHandleCountDbg());
}

/// Make sure that related Handle-s can be assigned.
TEST_F(HandleTest, ScopedPointerConstructorTest) {
  auto function = runtime.makeHandle<JSFunction>(
      JSFunction::create(runtime, domain, runtime.makeNullHandle<JSObject>()));
  Handle<JSObject> obj = function;
  ASSERT_EQ(function.get(), obj.get());
}

#if defined(HERMES_SLOW_DEBUG) && defined(ASSERT_DEATH)
TEST_F(HandleDeathTest, UseFlushedHandle) {
  auto marker = gcScope.createMarker();
  auto handle = StringPrimitive::createNoThrow(runtime, "hello");
  // This flush should destroy the handle created after the marker, using it
  // afterwards should trigger an assertion failure.
  gcScope.flushToMarker(marker);
  ASSERT_DEATH(
      { handle->getStringLength(); }, "Assertion.*Reading from flushed handle");
}

TEST_F(HandleDeathTest, UseRAIIFlushedHandle) {
  GCScopeMarkerRAII marker(gcScope);
  auto handle = StringPrimitive::createNoThrow(runtime, "hello");
  // This flush should destroy the handle created after the marker, using it
  // afterwards should trigger an assertion failure.
  marker.flush();
  ASSERT_DEATH(
      { handle->getStringLength(); }, "Assertion.*Reading from flushed handle");
}

TEST_F(HandleDeathTest, FlushToSmallCount) {
  auto handle = StringPrimitive::createNoThrow(runtime, "hello");
  // This flush should destroy the handle created after the marker, using it
  // afterwards should trigger an assertion failure.
  gcScope.flushToSmallCount(0);
  ASSERT_DEATH(
      { handle->getStringLength(); }, "Assertion.*Reading from flushed handle");
}
#endif

} // namespace
