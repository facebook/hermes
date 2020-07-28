/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ios>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <chrono>

#include <fbjni/fbjni.h>
#include <fbjni/JThread.h>

#include "expect.h"
#include "no_rtti.h"

#include "inter_dso_exception_test_2/Test.h"

#define EXPECT_SAME(A, B, C) EXPECT((A) == (B) && (B) == (C) && (C) == (A))

// A lot of the functions here are just confirming that compilation works.
#pragma GCC diagnostic ignored "-Wunused-function"

using namespace facebook::jni;

namespace {

struct Callbacks : public facebook::jni::JavaClass<Callbacks> {
  constexpr static auto kJavaDescriptor = "Lcom/facebook/jni/FBJniTests$Callbacks;";
};

struct TestThing : public JavaClass<TestThing> {
  constexpr static auto kJavaDescriptor = "Lcom/facebook/jni/FBJniTests$TestThing;";
};

// Yes, sloppy and does not handle conversions correctly but does it's job here
static std::string ToString(JNIEnv* env, jstring java_string) {
  auto chars = env->GetStringUTFChars(java_string, nullptr);
  if (chars == nullptr) {
    throw std::runtime_error{"Couldn't get UTF chars"};
  }

  std::string string{chars};
  env->ReleaseStringUTFChars(java_string, chars);

  return string;
}

jboolean TestClassResolution(JNIEnv* env, jobject self, jstring class_name) {
  auto resolved_class = findClassLocal(ToString(env, class_name).c_str());
  return resolved_class.get() != nullptr? JNI_TRUE : JNI_FALSE;
}

jboolean TestLazyClassResolution(JNIEnv* env, jobject self, jstring class_name) {
  auto resolved_class = alias_ref<jclass>{};
  resolved_class = findClassLocal(ToString(env, class_name).c_str());
  return resolved_class.get() != nullptr? JNI_TRUE : JNI_FALSE;
}

jobject TestCreateInstanceOf(JNIEnv* env, jobject self, jstring class_name) {
  auto clazz = findClassLocal(ToString(env, class_name).c_str());
  auto constructor = clazz->getConstructor<jobject(jstring)>();
  return clazz->newObject(constructor, class_name).release();
}

jboolean TestTypeDescriptors(JNIEnv* env, jobject self) {
#define FIXED_STRING_EXPECT_EQ(actual, expected)                        \
  static_assert((actual) == (expected), "descriptor mismatch")

  FIXED_STRING_EXPECT_EQ(jtype_traits<jboolean>::kDescriptor, "Z");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jbyte>::kDescriptor, "B");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jchar>::kDescriptor, "C");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jdouble>::kDescriptor, "D");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jfloat>::kDescriptor, "F");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jint>::kDescriptor, "I");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jlong>::kDescriptor, "J");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jshort>::kDescriptor, "S");

  FIXED_STRING_EXPECT_EQ(jtype_traits<jstring>::kDescriptor, "Ljava/lang/String;");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jobject>::kDescriptor, "Ljava/lang/Object;");

  FIXED_STRING_EXPECT_EQ(jtype_traits<jintArray>::kDescriptor, "[I");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jtypeArray<jstring>>::kDescriptor, "[Ljava/lang/String;");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jtypeArray<jtypeArray<jstring>>>::kDescriptor
     , "[[Ljava/lang/String;");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jtypeArray<jintArray>>::kDescriptor, "[[I");

  // base_name() is meaningless for primitive types.
  FIXED_STRING_EXPECT_EQ(jtype_traits<jstring>::kBaseName, "java/lang/String");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jobject>::kBaseName, "java/lang/Object");

  FIXED_STRING_EXPECT_EQ(jtype_traits<jintArray>::kBaseName, "[I");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jtypeArray<jstring>>::kBaseName, "[Ljava/lang/String;");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jtypeArray<jtypeArray<jstring>>>::kBaseName, "[[Ljava/lang/String;");
  FIXED_STRING_EXPECT_EQ(jtype_traits<jtypeArray<jintArray>>::kBaseName, "[[I");

  return JNI_TRUE;
}

jboolean TestVirtualMethodResolution_I(
    JNIEnv* env,
    jobject self,
    jstring class_name,
    jstring method_name)
{
  auto resolved_class = findClassLocal(ToString(env, class_name).c_str());
  auto resolved_method =
    resolved_class->getMethod<jint()>(ToString(env, method_name).c_str());
  return static_cast<bool>(resolved_method);
}

jboolean TestVirtualMethodResolution_arrB(
    JNIEnv* env,
    jobject self,
    jstring class_name,
    jstring method_name)
{
  auto resolved_class = findClassLocal(ToString(env, class_name).c_str());
  auto resolved_method =
    resolved_class->getMethod<jbyteArray()>(ToString(env, method_name).c_str());
  return static_cast<bool>(resolved_method);
}

jboolean TestVirtualMethodResolution_S_arrS(
    JNIEnv* env,
    jobject self,
    jstring class_name,
    jstring method_name)
{
  auto resolved_class = findClassLocal(ToString(env, class_name).c_str());
  auto resolved_method =
    resolved_class->getMethod<jtypeArray<jstring>(jstring)>(ToString(env, method_name).c_str());
  return static_cast<bool>(resolved_method);
}

jboolean TestVirtualMethodResolution_arrarrS(
    JNIEnv* env,
    jobject self,
    jstring class_name,
    jstring method_name)
{
  auto resolved_class = findClassLocal(ToString(env, class_name).c_str());
  auto resolved_method =
    resolved_class->getStaticMethod<jtypeArray<jtypeArray<jstring>>()>(ToString(env, method_name).c_str());
  return static_cast<bool>(resolved_method);
}

jboolean TestVirtualMethodResolution_arrarrI(
    JNIEnv* env,
    jobject self,
    jstring class_name,
    jstring method_name)
{
  auto resolved_class = findClassLocal(ToString(env, class_name).c_str());
  auto resolved_method =
    resolved_class->getStaticMethod<jtypeArray<jintArray>()>(ToString(env, method_name).c_str());
  return static_cast<bool>(resolved_method);
}

jboolean TestLazyVirtualMethodResolution_I(
    JNIEnv* env,
    jobject self,
    jstring class_name,
    jstring method_name)
{
  auto resolved_class = findClassLocal(ToString(env, class_name).c_str());
  auto resolved_method = JMethod<jint()>{};
  resolved_method = resolved_class->getMethod<jint()>(ToString(env, method_name).c_str());
  return static_cast<bool>(resolved_method);
}

void TestJMethodCallbacks(JNIEnv* env, jobject self, Callbacks::javaobject callbacks) {
  static const auto callbacks_class = Callbacks::javaClassStatic();

  static const auto void_foo = callbacks_class->getMethod<void()>("voidFoo");
  void_foo(callbacks);

  static const auto boolean_foo = callbacks_class->getMethod<jboolean()>("booleanFoo");
  boolean_foo(callbacks);

  static const auto byte_foo = callbacks_class->getMethod<jbyte()>("byteFoo");
  byte_foo(callbacks);

  static const auto char_foo = callbacks_class->getMethod<jchar()>("charFoo");
  char_foo(callbacks);

  static const auto short_foo = callbacks_class->getMethod<jshort()>("shortFoo");
  short_foo(callbacks);

  static const auto int_foo = callbacks_class->getMethod<jint()>("intFoo");
  int_foo(callbacks);

  static const auto long_foo = callbacks_class->getMethod<jlong()>("longFoo");
  long_foo(callbacks);

  static const auto float_foo = callbacks_class->getMethod<jfloat()>("floatFoo");
  float_foo(callbacks);

  static const auto double_foo = callbacks_class->getMethod<jdouble()>("doubleFoo");
  double_foo(callbacks);

  static const auto object_foo = callbacks_class->getMethod<jobject()>("objectFoo");
  object_foo(callbacks);

  static const auto string_foo = callbacks_class->getMethod<jstring()>("stringFoo");
  string_foo(callbacks);
}

// Try to test the static functions
void TestJStaticMethodCallbacks(JNIEnv* env, jobject self) {
  // static auto callbacks_class = findClassStatic(callbacks_class_name);
  auto cls = findClassLocal("com/facebook/jni/FBJniTests");
  jclass jcls = env->FindClass("com/facebook/jni/FBJniTests");

  static const auto void_foo_static = cls->getStaticMethod<void()>("voidFooStatic");
  void_foo_static(jcls);

  static const auto boolean_foo_static = cls->getStaticMethod<jboolean()>("booleanFooStatic");
  boolean_foo_static(jcls);

  static const auto byte_foo_static = cls->getStaticMethod<jbyte()>("byteFooStatic");
  byte_foo_static(jcls);

  static const auto char_foo_static = cls->getStaticMethod<jchar(jchar, jint)>("charFooStatic");
  char_foo_static(jcls, 'c', 5);

  static const auto short_foo_static = cls->getStaticMethod<jshort(jshort, jshort)>("shortFooStatic");
  short_foo_static(jcls, 17, 42);

  static const auto int_foo_static = cls->getStaticMethod<jint(jint)>("intFooStatic");
  int_foo_static(jcls, 5);

  static const auto long_foo_static = cls->getStaticMethod<jlong()>("longFooStatic");
  long_foo_static(jcls);

  static const auto float_foo_static = cls->getStaticMethod<jfloat()>("floatFooStatic");
  float_foo_static(jcls);

  static const auto double_foo_static = cls->getStaticMethod<jdouble()>("doubleFooStatic");
  double_foo_static(jcls);

  static const auto object_foo_static = cls->getStaticMethod<jobject()>("objectFooStatic");
  object_foo_static(jcls);

  static const auto string_foo_static = cls->getStaticMethod<jstring()>("stringFooStatic");
  string_foo_static(jcls);
}

jboolean TestIsAssignableFrom(JNIEnv* env, jobject self, jclass cls1, jclass cls2) {
  return adopt_local(cls1)->isAssignableFrom(cls2);
}

jboolean TestIsInstanceOf(JNIEnv* env, jobject self, jobject test_object, jclass cls) {
  auto clsref = adopt_local(test_object);
  return clsref->isInstanceOf(cls);
}

jboolean TestIsSameObject(JNIEnv* env, jobject self, jobject a, jobject b) {
  return isSameObject(a, b);
}

jboolean TestGetSuperclass(JNIEnv* env, jobject self, jclass test_class, jclass super_class) {
  return isSameObject(adopt_local(test_class)->getSuperclass().get(), super_class);
}

jboolean StaticCastAliasRefToString(JNIEnv *, jobject , jobject string_as_object) {
  alias_ref<jobject> string_as_object_alias_ref { string_as_object };
  alias_ref<jstring> string_alias_ref = static_ref_cast<jstring>(string_as_object_alias_ref);
  return isSameObject(string_alias_ref.get(), string_as_object_alias_ref.get());
}

jboolean DynamicCastAliasRefToThrowable(JNIEnv *, jobject , jobject might_actually_be_throwable) {
  alias_ref<jobject> might_actually_be_throwable_alias_ref { might_actually_be_throwable };
  // If the next line fails, it will throw an exception.
  alias_ref<jthrowable> throwable_alias_ref = dynamic_ref_cast<jthrowable>(might_actually_be_throwable_alias_ref);
  return isSameObject(throwable_alias_ref.get(), might_actually_be_throwable_alias_ref.get());
}

jboolean StaticCastLocalRefToString(JNIEnv *, jobject , jobject string_as_object) {
  local_ref<jobject> string_as_object_local_ref = adopt_local(string_as_object);
  local_ref<jstring> string_local_ref = static_ref_cast<jstring>(string_as_object_local_ref);
  return isSameObject(string_local_ref.get(), string_as_object_local_ref.get());
}

jboolean DynamicCastLocalRefToString(JNIEnv *, jobject , jobject might_actually_be_string) {
  local_ref<jobject> might_actually_be_string_local_ref = adopt_local(might_actually_be_string);
  // If the next line fails, it will throw an exception.
  local_ref<jstring> string_local_ref = dynamic_ref_cast<jstring>(might_actually_be_string_local_ref);
  return isSameObject(string_local_ref.get(), might_actually_be_string_local_ref.get());
}

jboolean StaticCastGlobalRefToString(JNIEnv *, jobject , jobject string_as_object) {
  global_ref<jobject> string_as_object_global_ref = make_global(string_as_object);
  global_ref<jstring> string_global_ref = static_ref_cast<jstring>(string_as_object_global_ref);
  return isSameObject(string_global_ref.get(), string_as_object_global_ref.get());
}

jboolean DynamicCastGlobalRefToString(JNIEnv *, jobject , jobject might_actually_be_string) {
  global_ref<jobject> might_actually_be_string_global_ref = make_global(might_actually_be_string);
  // If the next line fails, it will throw an exception.
  global_ref<jstring> string_global_ref = dynamic_ref_cast<jstring>(might_actually_be_string_global_ref);
  return isSameObject(string_global_ref.get(), might_actually_be_string_global_ref.get());
}

template<typename... Args>
static void Use(Args&&... args) {}

jboolean TestWeakRefs(JNIEnv*, jobject self) {
  using facebook::jni::internal::g_reference_stats;

  g_reference_stats.reset();
  {
    // Wrapping existing local that should be deleted (locals = 1)
    auto local = adopt_local(self);
    // Make new local (locals = 2)
    auto local2 = make_local(local);
    // Make weak (weaks = 1)
    auto weak = make_weak(local);
    // Make global (globals = 1)
    auto global = weak.lockGlobal();
    // No new refs
    auto banana = std::move(weak);
    // No new refs
    auto& binini = banana;
    // Create a global of the local (keeping the local) (globals = 2)
    auto dubglob = make_global(local);
    // Create a weak (weaks = 2)
    auto dupweak = make_weak(local);
    // No new refs
    swap(local, local2);

    Use(binini);
  }

  FBJNI_LOGE("locals: %d", g_reference_stats.locals_deleted.load());
  FBJNI_LOGE("globals: %d", g_reference_stats.globals_deleted.load());
  FBJNI_LOGE("weaks: %d", g_reference_stats.weaks_deleted.load());

  return (g_reference_stats.locals_deleted == 2 &&
          g_reference_stats.globals_deleted == 2 &&
          g_reference_stats.weaks_deleted == 2)? JNI_TRUE : JNI_FALSE;
}

jboolean TestAlias(JNIEnv* env, jobject self) {
  auto ref = alias_ref<jobject>{self};
  return ref->isInstanceOf(findClassLocal("java/lang/Object"));
}

jboolean testAliasRefConversions(JNIEnv*, jobject self) {
  auto aLocalString = make_jstring("foo");
  alias_ref<jstring> aString = aLocalString;
  alias_ref<jobject> anObject = aLocalString;
  anObject = (jstring) nullptr;
  anObject = aString;
  // aString = anObject; // Shouldn't compile

  return isSameObject(aString, anObject)? JNI_TRUE : JNI_FALSE;
}

void TestAutoAliasRefReturningVoid(facebook::jni::alias_ref<jobject> self) {
  // If this compiles, it succeeds.
}

jboolean testNullJString(JNIEnv*, jobject) {
  auto aNullJString = make_jstring(nullptr);
  EXPECT(aNullJString.get() == (jstring) nullptr);
  return JNI_TRUE;
}

jboolean testSwap(JNIEnv*, jobject self, jobject other) {
  auto selfAlias = wrap_alias(self);
  auto otherAlias = wrap_alias(other);

  swap(selfAlias, otherAlias);
  EXPECT(self == otherAlias);
  EXPECT(other == selfAlias);
  EXPECT(self != selfAlias);
  EXPECT(other != otherAlias);

  auto selfLocal = make_local(self);
  auto otherLocal = make_local(other);
  swap(selfLocal, otherLocal);
  EXPECT(self == otherLocal);
  EXPECT(other == selfLocal);
  EXPECT(self != selfLocal);
  EXPECT(other != otherLocal);

  auto selfGlobal = make_global(self);
  auto otherGlobal = make_global(other);
  swap(selfGlobal, otherGlobal);
  EXPECT(self == otherGlobal);
  EXPECT(other == selfGlobal);
  EXPECT(self != selfGlobal);
  EXPECT(other != otherGlobal);

  auto selfWeak = make_weak(self);
  auto otherWeak = make_weak(other);
  swap(selfWeak, otherWeak);
  auto selfLockedWeak = selfWeak.lockLocal();
  auto otherLockedWeak = otherWeak.lockLocal();
  EXPECT(self == otherLockedWeak);
  EXPECT(other == selfLockedWeak);
  EXPECT(self != selfLockedWeak);
  EXPECT(other != otherLockedWeak);

  return JNI_TRUE;
}

jboolean testEqualOperator(JNIEnv*, jobject self, jobject other) {
  auto selfAlias = wrap_alias(self);
  auto otherAlias = wrap_alias(other);
  auto selfLocal = adopt_local(self);
  auto otherLocal = adopt_local(other);
  auto selfGlobal = make_global(self);
  auto otherGlobal = make_global(other);
  auto selfWeak = make_weak(self);
  auto otherWeak = make_weak(other);
  auto selfLockedWeak = selfWeak.lockLocal();
  auto otherLockedWeak = otherWeak.lockLocal();

  EXPECT(self == selfAlias);
  EXPECT(selfAlias == selfLocal);
  EXPECT(selfLocal == selfGlobal);
  EXPECT(selfGlobal == selfLockedWeak);
  EXPECT(self != other);
  EXPECT(self != otherAlias);
  EXPECT(self != otherLocal);
  EXPECT(self != otherGlobal);
  EXPECT(self != otherLockedWeak);
  EXPECT(selfAlias != nullptr);
  EXPECT(!(selfAlias == nullptr));
  EXPECT(nullptr != selfLocal);
  EXPECT(!(nullptr == selfGlobal));

  return JNI_TRUE;
}

jboolean testReleaseAlias(JNIEnv*, jobject self) {
  auto local = adopt_local(self);
  auto alias = local.releaseAlias();

  EXPECT(typeid(alias) == typeid(alias_ref<jobject>));
  EXPECT(isSameObject(self, alias.get()));

  return JNI_TRUE;
}

jboolean testLockingWeakReferences(JNIEnv*, jobject self) {
  auto weak = make_weak(self);
  auto local = weak.lockLocal();
  auto global = weak.lockGlobal();

  EXPECT(typeid(local) == typeid(local_ref<jobject>));
  EXPECT(typeid(global) == typeid(global_ref<jobject>));
  EXPECT(self == local);
  EXPECT(self == global);

  return JNI_TRUE;
}

jboolean TestFieldAccess(alias_ref<jobject> self, const std::string& field_name,
                         jint oldval, jint newval) {
  auto cls = self->getClass();
  auto fld = cls->getField<jint>(field_name.c_str());
  auto method = cls->getMethod<jint(jdouble)>("bar");

  if (method(self.get(), 17) != 42) {
    return JNI_FALSE;
  }

  if (method(self, 17) != 42) {
    return JNI_FALSE;
  }

  if (self->getFieldValue(fld) != oldval) {
    return JNI_FALSE;
  }

  self->setFieldValue(fld, newval);

  return JNI_TRUE;
}

jboolean TestStringFieldAccess(
    JNIEnv* env,
    jobject self,
    jstring field_name,
    jstring oldval,
    jstring newval) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto fld = cls->getField<jstring>(ToString(env, field_name).c_str());
  auto oldvalStr = adopt_local(oldval)->toStdString();

  auto curvalRef = me->getFieldValue(fld);
  if (curvalRef->toStdString() != oldvalStr) {
    return JNI_FALSE;
  }

  const alias_ref<jobject> cme = me;
  if (cme->getFieldValue(fld)->toStdString() != oldvalStr) {
    return JNI_FALSE;
  }

  me->setFieldValue(fld, newval);

  return JNI_TRUE;
}

jboolean TestReferenceFieldAccess(
    alias_ref<jobject> self,
    std::string const& field_name,
    jobject oldval,
    jobject newval,
    jboolean useWrapper) {
  auto cls = self->getClass();
  auto rawfld = cls->getField<jobject>(field_name.c_str(), TestThing::kJavaDescriptor);

  if (self->getFieldValue(rawfld) != oldval) {
    return JNI_FALSE;
  }

  alias_ref<jobject> const cself = self;
  if (cself->getFieldValue(rawfld) != oldval) {
    return JNI_FALSE;
  }

  if (useWrapper) {
    auto newvalRef = adopt_local(static_cast<TestThing::javaobject>(newval));
    auto fld = cls->getField<TestThing::javaobject>(field_name.c_str());
    self->setFieldValue<TestThing::javaobject>(fld, newvalRef);
  } else {
    self->setFieldValue(rawfld, newval);
  }

  return JNI_TRUE;
}

jboolean TestStaticFieldAccess(
    JNIEnv* env,
    jobject self,
    jstring field_name,
    jint oldval,
    jint newval) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto fld = cls->getStaticField<jint>(ToString(env, field_name).c_str());

  if (cls->getStaticFieldValue(fld) != oldval) {
    return JNI_FALSE;
  }
  cls->setStaticFieldValue(fld, newval);
  return JNI_TRUE;
}

jboolean TestStaticStringFieldAccess(
    JNIEnv* env,
    jobject self,
    jstring field_name,
    jstring oldval,
    jstring newval) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto fld = cls->getStaticField<jstring>(ToString(env, field_name).c_str());

  auto curvalRef = cls->getStaticFieldValue(fld);
  if (curvalRef->toStdString() != adopt_local(oldval)->toStdString()) {
    return JNI_FALSE;
  }
  cls->setStaticFieldValue(fld, newval);
  return JNI_TRUE;
}

jboolean TestStaticReferenceFieldAccess(
    alias_ref<jobject> self,
    std::string const& field_name,
    jobject oldval,
    jobject newval,
    jboolean useWrapper) {
  auto cls = self->getClass();
  auto rawfld = cls->getStaticField<jobject>(field_name.c_str(), TestThing::kJavaDescriptor);

  auto curvalRef = cls->getStaticFieldValue(rawfld);
  if (curvalRef != oldval) {
    return JNI_FALSE;
  }

  if (useWrapper) {
    auto newvalRef = adopt_local(static_cast<TestThing::javaobject>(newval));
    auto fld = cls->getStaticField<TestThing::javaobject>(field_name.c_str());
    cls->setStaticFieldValue<TestThing::javaobject>(fld, newvalRef);
  } else {
    cls->setStaticFieldValue(rawfld, newval);
  }

  return JNI_TRUE;
}

jboolean TestNonVirtualMethod(JNIEnv* env, jobject self, jboolean s) {
  auto me = adopt_local(self);
  if (!me) {
    return JNI_FALSE;
  }

  auto cls = me->getClass();
  if (!cls) {
    return JNI_FALSE;
  }
  auto method = cls->getNonvirtualMethod<jboolean(jboolean)>("nonVirtualMethod");

  jclass jcls = env->FindClass("com/facebook/jni/FBJniTests");

  return method(self, jcls, s);
}

jtypeArray<jstring>
TestArrayCreation(JNIEnv* env, jobject self, jstring s0, jstring s1, jstring s2) {
  auto array = JArrayClass<jstring>::newArray(3);
  array->setElement(0, s0);
  array->setElement(1, s1);
  array->setElement(2, s2);
  return static_cast<jtypeArray<jstring>>(array.release());
}

jtypeArray<jtypeArray<jstring>>
TestMultidimensionalObjectArray(JNIEnv* env, jobject self, jstring s0, jstring s1, jstring s2) {
  auto array = JArrayClass<jtypeArray<jstring>>::newArray(2);
  auto row = JArrayClass<jstring>::newArray(2);
  row->setElement(0, s0);
  row->setElement(1, s1);
  (*array)[0] = row;
  row = JArrayClass<jstring>::newArray(1);
  row->setElement(0, s2);
  (*array)[1] = row;
  return array.release();
}

jtypeArray<jintArray>
TestMultidimensionalPrimitiveArray(JNIEnv* env, jobject self, jint i0, jint i1, jint i2) {
  auto array = JArrayClass<jintArray>::newArray(2);
  auto row = JArrayInt::newArray(2);
  row->setRegion(0, 1, &i0);
  row->setRegion(1, 1, &i1);
  (*array)[0] = row;
  row = JArrayInt::newArray(1);
  row->setRegion(0, 1, &i2);
  (*array)[1] = row;
  return array.release();
}

jstring TestBuildStringArray(JNIEnv* env, jobject self, jtypeArray<jstring> input) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto method = cls->getMethod<jstring(jtypeArray<jstring>)>("captureStringArray");

  auto niceInput = adopt_local_array<jstring>(input);
  auto length = niceInput->size();
  auto inputCopy = JArrayClass<jstring>::newArray(length);
  for (size_t idx = 0; idx < length; idx++) {
    switch (idx % 3) {
    case 0: {
      // Verify that assignment from a T works.
      jstring value = (jstring)env->GetObjectArrayElement(input, idx);
      (*inputCopy)[idx] = value; // Assignment from actual type.
      env->DeleteLocalRef(value);
      break;
    }
    case 1: {
      // Verify that direct assignment from an ElementProxy works.
      (*inputCopy)[idx] = (*niceInput)[idx];
      break;
    }
    case 2:
    default: {
      // Verify that assignment from a smart reference works.
      auto smartRef = adopt_local((*niceInput)[idx]);
      (*inputCopy)[idx] = smartRef;
      break;
    }
    }
  }

  return method(self, inputCopy.get()).release();
}

template <typename F, typename... Args>
void tryResolveMethodWithCxxTypes(std::string sig, alias_ref<jobject> me, std::string methodName, Args... args) {
  auto cls = me->getClass();
  auto method = cls->getMethod<F>(methodName.c_str());
  if (!method) throw std::runtime_error("method lookup failed with signature=" + sig);
  try {
    method(me, args...);
  } catch (std::exception&) {
    throw std::runtime_error("calling method failed with signature=" + sig);
  }

  auto nonVirtualMethod = cls->getNonvirtualMethod<F>(methodName.c_str());
  if (!nonVirtualMethod) throw std::runtime_error("method lookup failed with signature=" + sig);
  try {
    nonVirtualMethod(me, cls.get(), args...);
  } catch (std::exception&) {
    throw std::runtime_error("calling method failed with signature=" + sig);
  }

  auto staticMethod = cls->getStaticMethod<F>((methodName + "Static").c_str());
  if (!staticMethod) throw std::runtime_error("static method lookup failed with signature=" + sig);
  try {
    staticMethod(cls, args...);
  } catch (std::exception&) {
    throw std::runtime_error("calling static method failed with signature=" + sig);
  }
}

// Simple utility to give us a good error message.
#define runTest(sig, ...) \
  tryResolveMethodWithCxxTypes<sig>(#sig, self, method, __VA_ARGS__);

void TestMethodResolutionWithCxxTypes(alias_ref<jobject> self, alias_ref<jstring> jmethod, alias_ref<jstring> str, jlong v) {
  auto method = jmethod->toStdString();
  runTest(jobject(jstring, jlong), str.get(), v);
  runTest(local_ref<jobject>(jstring, jlong), str.get(), v);

  runTest(jobject(local_ref<jstring>, jlong), make_local(str), v);
  runTest(jobject(alias_ref<jstring>, jlong), str, v);

  runTest(jobject(alias_ref<jstring>, int64_t), str, (int64_t)v);
  runTest(jobject(alias_ref<jstring>, long long), str, (long long)v);

  runTest(jobject(const char*, int64_t), str->toStdString().c_str(), (int64_t)v);

  method = jmethod->toStdString() + "Void";
  runTest(void(jstring, int64_t), str.get(), v);

  method = jmethod->toStdString() + "Int";
  runTest(jint(jstring, int64_t), str.get(), v);
}

#undef runTest

void TestHandleJavaCustomException(JNIEnv* env, jobject self) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto method = cls->getMethod<void()>("customExceptionThrower");

  method(self);
}

void TestHandleNullExceptionMessage(JNIEnv* env, jobject self) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto method = cls->getMethod<void()>("nullMessageThrower");

  try {
    method(self);
  } catch (const std::exception& ex) {
    ex.what();
  }
}

void TestHandleNestedException(JNIEnv* env, jobject self) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto method = cls->getMethod<void()>("customExceptionThrower");

  try {
    try {
      method(self);
    } catch (...) {
      std::throw_with_nested(std::runtime_error("middle"));
    }
  } catch (...) {
    std::throw_with_nested(std::out_of_range("outer"));
  }
}

void TestHandleNoRttiException(JNIEnv* env, jobject self) {
  nortti::throwException();
}

jstring TestCopyConstructor(JNIEnv* env, jobject self) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto method = cls->getMethod<void()>("customExceptionThrower");

  try {
    method(self);
    return env->NewStringUTF("method did not throw");
  } catch (JniException ex) { // No & -- we're intentionally invoking the copy constructor.
    return env->NewStringUTF(ex.what());
  }
}

jstring TestMoveConstructorWithEmptyWhat(JNIEnv* env, jobject self) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto method = cls->getMethod<void()>("customExceptionThrower");

  try {
    method(self);
    return env->NewStringUTF("method did not throw");
  } catch (JniException& ex) {
    auto replacement = JniException(std::move(ex));
    return env->NewStringUTF(replacement.what());
  }
}

jstring TestMoveConstructorWithPopulatedWhat(JNIEnv* env, jobject self) {
  auto me = adopt_local(self);
  auto cls = me->getClass();
  auto method = cls->getMethod<void()>("customExceptionThrower");

  try {
    method(self);
    return env->NewStringUTF("method did not throw");
  } catch (JniException& ex) {
    ex.what();
    auto replacement = JniException(std::move(ex));
    return env->NewStringUTF(replacement.what());
  }
}

void TestHandleCppRuntimeError(JNIEnv* env, jobject self, jstring message) {
  throw std::runtime_error(ToString(env, message));
}

void TestHandleCppIOBaseFailure(JNIEnv* env, jobject self) {
  throw std::ios_base::failure("A C++ IO base failure.");
}

void TestHandleCppSystemError(JNIEnv* env, jobject self) {
  // Throw a sample std::system_error
  throw std::system_error(EFAULT, std::system_category());
}

void TestInterDsoExceptionHandlingA(JNIEnv* env, jobject self) {
  inter_dso_exception_test_2a();
}

jboolean TestInterDsoExceptionHandlingB(JNIEnv* env, jobject self) {
  return inter_dso_exception_test_2b();
}

struct NonStdException /* has no base class */ {};

void TestHandleNonStdException(JNIEnv* env, jobject self) {
  throw NonStdException();
}

void TestHandleCppIntThrow(JNIEnv* env, jobject self) {
  throw 42;
}

void TestHandleCppCharPointerThrow(JNIEnv* env, jobject self) {
  throw "Some random message";
}

void TestThrowJavaExceptionByName(JNIEnv* env, jobject self) {
  throwNewJavaException("java/lang/IllegalArgumentException", "bad news: %s", "it didn't work");
}

jint TestJThread(JNIEnv* env, jobject self) {
  jint i = -1;
  auto thread = JThread::create([&] {
      i = 0;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      i = 1;
    });
  thread->start();
  thread->join();
  return i;
}

// Global for simpleWorker tests. Relies on thread sync points (constructor, join) for "locking".
jint gWorkerValue;

void simpleWorker(jobject grefSelf, jdouble input) {
  auto attachGuard = ThreadScope(); // This tests the move constructor.
  auto self = adopt_global(grefSelf);
  // Claw up from the object to avoid classloader issues.
  auto barMethod = self->getClass()->getMethod<jint(jdouble)>("bar");
  gWorkerValue = barMethod(self.get(), input);
}

void nestedSimpleWorker(jobject grefSelf, jdouble input) {
  ThreadScope attachGuard; // More efficient version of guard; no move constructor required.
  simpleWorker(grefSelf, input);
}

jint TestThreadScopeGuard(JNIEnv* env, jobject self, jdouble input) {
  // Turn self into a global reference before passing it to a working thread.
  auto grefSelf = make_global(adopt_local(self));
  auto childThread = std::thread(simpleWorker, grefSelf.release(), input);
  childThread.join();
  return gWorkerValue;
}

jint TestNestedThreadScopeGuard(JNIEnv* env, jobject self, jdouble input) {
  // Turn self into a global reference before passing it to a working thread.
  auto grefSelf = make_global(adopt_local(self));
  auto childThread = std::thread(nestedSimpleWorker, grefSelf.release(), input);
  childThread.join();
  return gWorkerValue;
}

void classLoadWorker() {
  gWorkerValue = 0;
  try {
    // This should fail because we aren't attached.
    Callbacks::javaClassLocal();
    gWorkerValue = -1;
    return;
  } catch (std::exception& e) {
    // ignored
  }
  try {
    ThreadScope::WithClassLoader([&] {
      // This should now succeed.
      Callbacks::javaClassLocal();
      gWorkerValue = 1;
    });
  } catch (std::exception& e) {
    gWorkerValue = -2;
    // Catch this and log it so that we get a test failure instead of a crash.
    FBJNI_LOGE("%s", e.what());
  }
}

jint TestClassLoadInWorker(JNIEnv* env, jobject self) {
  std::thread t(classLoadWorker);
  t.join();
  return gWorkerValue;
}

jint TestClassLoadWorkerFastPath(JNIEnv* env, jobject self) {
  jint i = 0;
  ThreadScope::WithClassLoader([&] {
    // Execute on the fast path
    Callbacks::javaClassLocal();
    i += 1;
  });

  std::thread t([&] {
    ThreadScope::WithClassLoader([&] {
      // Execute on the slow path
      Callbacks::javaClassLocal();
      i += 1;
    });
  });
  t.join();

  std::thread t2([&] {
    ThreadScope scope;
    ThreadScope::WithClassLoader([&] {
      // Execute on the slow path even though thread is already attached.
      Callbacks::javaClassLocal();
      i += 1;
    });
  });
  t2.join();

  return i;
}

void testNewObject(JNIEnv*, jobject self) {
  // This is a compilation only test, verifies that all the types work out.
  auto cls = findClassLocal("java/lang/String");
  auto ctr = cls->getConstructor<jstring()>();
  local_ref<jstring> obj = cls->newObject(ctr);
  auto str = obj->toStdString();
  Use(str);
}

template<typename T>
static jboolean copyAndVerify(T& orig) {
  T copy{orig};
  EXPECT(orig == copy);

  return JNI_TRUE;
}

template<typename T>
static jboolean assignAndVerify(T& orig) {
  T copy{};
  copy = orig;
  EXPECT(orig == copy);

  return JNI_TRUE;
}

jboolean testNullReferences(JNIEnv*, jobject) {
  jobject nullobject = nullptr;

  auto local = local_ref<jobject>{};
  EXPECT(!local);

  auto localWrap = adopt_local(nullobject);
  EXPECT(!localWrap);

  auto localMake = make_local(local);
  EXPECT(!localMake);
  EXPECT_SAME(local, localWrap, localMake);

  auto global = global_ref<jobject>{};
  EXPECT(!global);

  auto globalWrap = adopt_global(nullobject);
  EXPECT(!globalWrap);

  auto globalMake = make_global(global);
  EXPECT(!globalMake);
  EXPECT_SAME(global, globalWrap, globalMake);

  weak_ref<jobject> weak_global = weak_ref<jobject>{};
  EXPECT(!weak_global.lockLocal());

  weak_ref<jobject> weak_globalWrap = adopt_weak_global(nullobject);
  EXPECT(!weak_globalWrap.lockLocal());
  EXPECT(weak_global.lockLocal() == weak_globalWrap.lockGlobal());
  EXPECT(!make_local(adopt_local(nullobject)));
  EXPECT(!make_global(nullobject));

  return JNI_TRUE;
}

jboolean testCreatingReferences(JNIEnv*, jobject self) {
  auto a = wrap_alias(self);
  auto l = adopt_local(self);
  auto g = make_global(l);
  auto w = make_weak(l);

  EXPECT(a == l && a == g && a == w.lockLocal());

  auto lp = make_local(self);
  auto la = make_local(a);
  auto ll = make_local(l);
  auto lg = make_local(g);

  EXPECT(a == lp && a == la && a == ll && a == lg);

  auto gp = make_global(self);
  auto ga = make_global(a);
  auto gl = make_global(l);
  auto gg = make_global(g);

  EXPECT(a == gp && a == ga && a == gl && a == gg);

  return JNI_TRUE;
}

jboolean testAssignmentAndCopyConstructors(JNIEnv*, jobject self) {
  using facebook::jni::internal::g_reference_stats;

  g_reference_stats.reset();
  {
    // Wrapping existing local that should be deleted (locals = 1)
    auto local = adopt_local(self);
    // Copy constructor (locals = 2)
    EXPECT(copyAndVerify(local));

    // Assignment (locals = 3)
    EXPECT(assignAndVerify(local));

    // Creating a new global (globals = 1)
    auto global = make_global(local);
    // Copy constructor (globals = 2)
    EXPECT(copyAndVerify(global));

    // Assignment (globals = 3)
    EXPECT(assignAndVerify(global));

    // Creating a new weak (weaks = 1)
    auto weak = make_weak(local);
    // Copy constructor (weaks = 2, globals = 5)
    weak_ref<jobject> weakCopy{weak};
    EXPECT(weak.lockGlobal() == weakCopy.lockGlobal());

    // Assignment (weaks = 3, globals = 7)
    weakCopy = weak;
    EXPECT(weak.lockGlobal() == weakCopy.lockGlobal());

    auto alias = alias_ref<jobject>{local};
    alias_ref<jobject> aliasCopy{alias};
    EXPECT(alias == aliasCopy);

    aliasCopy = alias;
    EXPECT(alias == aliasCopy);

    alias = self;
    alias = global;
    // alias = weak; // Should not compile
  }

  FBJNI_LOGE("locals: %d", g_reference_stats.locals_deleted.load());
  FBJNI_LOGE("globals: %d", g_reference_stats.globals_deleted.load());
  FBJNI_LOGE("weaks: %d", g_reference_stats.weaks_deleted.load());

  EXPECT(g_reference_stats.locals_deleted == 3 &&
        g_reference_stats.globals_deleted == 7 &&
        g_reference_stats.weaks_deleted == 3);

  return JNI_TRUE;
}

template<template <typename> class RefType, typename T>
static jboolean copyAndVerifyCross(RefType<T>& orig) {
  RefType<ReprType<T>> reprCopy{orig};
  RefType<JniType<T>> jniCopy{orig};
  EXPECT(orig == reprCopy);
  EXPECT(orig == jniCopy);
  return JNI_TRUE;
}

template<template <typename> class RefType, typename T>
static jboolean assignAndVerifyCross(RefType<T>& orig) {
  RefType<ReprType<T>> reprCopy{};
  reprCopy = orig;
  RefType<JniType<T>> jniCopy{};
  jniCopy = orig;
  EXPECT(orig == reprCopy);
  EXPECT(orig == jniCopy);
  return JNI_TRUE;
}

template<template <typename> class RefType, typename T>
static jboolean verifyMakeCross(RefType<T>& orig) {
  RefType<ReprType<T>> copy{orig};
  {
    local_ref<T> local = make_local(copy);
    global_ref<T> global = make_global(copy);
    weak_ref<T> weak = make_weak(copy);
  }

  {
    local_ref<ReprType<T>> local = make_local(copy);
    global_ref<ReprType<T>> global = make_global(copy);
    weak_ref<ReprType<T>> weak = make_weak(copy);
  }

  {
    local_ref<T> local = make_local(orig);
    global_ref<T> global = make_global(orig);
    weak_ref<T> weak = make_weak(orig);
  }

  {
    local_ref<ReprType<T>> local = make_local(orig);
    global_ref<ReprType<T>> global = make_global(orig);
    weak_ref<ReprType<T>> weak = make_weak(orig);
  }

  return JNI_TRUE;
}


jboolean testAssignmentAndCopyCrossTypes(JNIEnv*, jobject self) {
  using facebook::jni::internal::g_reference_stats;

  size_t locals = 0, globals = 0, weaks = 0;
  g_reference_stats.reset();
#define VERIFY_REFERENCE_STATS() do {                                   \
    bool referenceStatsMatch = g_reference_stats.locals_deleted == locals && \
      g_reference_stats.globals_deleted == globals &&                   \
      g_reference_stats.weaks_deleted == weaks;                         \
    if (!referenceStatsMatch) {                                         \
      FBJNI_LOGE("locals: %d, expected: %zd", g_reference_stats.locals_deleted.load(), locals); \
      FBJNI_LOGE("globals: %d, expected: %zd", g_reference_stats.globals_deleted.load(), globals); \
      FBJNI_LOGE("weaks: %d, expected: %zd", g_reference_stats.weaks_deleted.load(), weaks); \
    }                                                                   \
    EXPECT(referenceStatsMatch);                                        \
  } while (0)

  {
    VERIFY_REFERENCE_STATS();

    auto local = adopt_local(self);
    VERIFY_REFERENCE_STATS();

    EXPECT(copyAndVerifyCross<local_ref>(local));
    locals += 2;
    VERIFY_REFERENCE_STATS();

    EXPECT(assignAndVerifyCross<local_ref>(local));
    locals += 2;
    VERIFY_REFERENCE_STATS();

    EXPECT(verifyMakeCross<local_ref>(local));
    locals += 1;
    locals += 4;
    globals += 4;
    weaks += 4;
    VERIFY_REFERENCE_STATS();

    auto global = make_global(local);
    VERIFY_REFERENCE_STATS();

    EXPECT(copyAndVerifyCross<global_ref>(global));
    globals += 2;
    VERIFY_REFERENCE_STATS();

    EXPECT(assignAndVerifyCross<global_ref>(global));
    globals += 2;
    VERIFY_REFERENCE_STATS();

    auto weak = make_weak(local);
    VERIFY_REFERENCE_STATS();

    weak_ref<JObject> weakCopy{weak};
    VERIFY_REFERENCE_STATS();

    EXPECT(weak.lockGlobal() == weakCopy.lockGlobal());
    globals += 2;
    VERIFY_REFERENCE_STATS();

    weakCopy = weak;
    weaks += 1;
    EXPECT(weak.lockGlobal() == weakCopy.lockGlobal());
    globals += 2;
    VERIFY_REFERENCE_STATS();

    auto alias = alias_ref<jobject>{local};
    alias_ref<JObject>{local};

    alias_ref<JObject> aliasCopy{alias};
    EXPECT(alias == aliasCopy);

    aliasCopy = alias;
    alias = aliasCopy;
    EXPECT(alias == aliasCopy);

    alias = self;
    alias = global;
    // alias = weak; // Should not compile

    weaks += 1;   // `weakCopy` going out of scope
    weaks += 1;   // `weak` going out of scope
    globals += 1; // `global` going out of scope
    locals += 1;  // `local` going out of scope
  }

  VERIFY_REFERENCE_STATS();

  return JNI_TRUE;

}

jboolean testToString(JNIEnv* env, jobject self) {
  auto dateClass = findClassLocal("java/util/Date");
  auto dateConstructor = dateClass->getConstructor<jobject()>();
  auto date = dateClass->newObject(dateConstructor);

  auto objectClass = findClassLocal("java/lang/Object");
  auto objectConstructor = objectClass->getConstructor<jobject()>();
  auto object = objectClass->newObject(objectConstructor);

  // First call the date implementation of toString
  auto dateString = date->toString();
  // And ensure that we don't use Date's toString method when calling
  // toString on the object. If this doesn't crash we are fine.
  auto objectString = object->toString();

  return JNI_TRUE;
}

jboolean testCriticalNativeMethodBindsAndCanBeInvoked(jint a, jfloat b) {
  return JNI_TRUE;
}

// These implicit nullptr tests aren't called, the test is that it
// compiles.
alias_ref<JObject> returnNullAliasRef() {
  return nullptr;
}

local_ref<JObject> returnNullLocalRef() {
  return nullptr;
}

global_ref<JObject> returnNullGlobalRef() {
  return nullptr;
}

void takesAliasRef(alias_ref<JObject>) {}
void takesLocalRef(local_ref<JObject>) {}
void takesGlobalRef(global_ref<JObject>) {}

void callWithNullRefs() {
  takesAliasRef(nullptr);
  takesLocalRef(nullptr);
  takesGlobalRef(nullptr);
}

struct SomeJavaFoo;
struct OtherJavaFoo;

struct SomeJavaFoo : JavaClass<SomeJavaFoo> {
  // Ensure that smart references can be used in declarations using forward-declared types.
  alias_ref<OtherJavaFoo> call(alias_ref<SomeJavaFoo>);
  static alias_ref<OtherJavaFoo> funcWithForwardDeclaredRefs(local_ref<OtherJavaFoo> foo);
};


using sjf = SomeJavaFoo::javaobject;

static_assert(IsNonWeakReference<local_ref<jobject>>(), "");
static_assert(IsNonWeakReference<local_ref<sjf>>(), "");
static_assert(IsNonWeakReference<global_ref<jobject>>(), "");
static_assert(IsNonWeakReference<jobject>(), "");
static_assert(IsNonWeakReference<sjf>(), "");
static_assert(IsNonWeakReference<jintArray>(), "");
static_assert(IsNonWeakReference<jbooleanArray>(), "");

static_assert(!IsNonWeakReference<weak_ref<jobject>>(), "");
static_assert(!IsNonWeakReference<weak_ref<sjf>>(), "");
static_assert(!IsNonWeakReference<std::string>(), "");
static_assert(!IsNonWeakReference<jint*>(), "");
static_assert(!IsNonWeakReference<void>(), "");
static_assert(!IsNonWeakReference<int>(), "");


static_assert(IsAnyReference<local_ref<jobject>>(), "");
static_assert(IsAnyReference<local_ref<sjf>>(), "");
static_assert(IsAnyReference<global_ref<jobject>>(), "");
static_assert(IsAnyReference<jobject>(), "");
static_assert(IsAnyReference<sjf>(), "");
static_assert(IsAnyReference<jintArray>(), "");
static_assert(IsAnyReference<jbooleanArray>(), "");
static_assert(IsAnyReference<weak_ref<jobject>>(), "");
static_assert(IsAnyReference<weak_ref<sjf>>(), "");

static_assert(!IsAnyReference<std::string>(), "");
static_assert(!IsAnyReference<jint*>(), "");
static_assert(!IsAnyReference<void>(), "");
static_assert(!IsAnyReference<int>(), "");


static_assert(IsPlainJniReference<jobject>(), "");
static_assert(IsPlainJniReference<sjf>(), "");
static_assert(IsPlainJniReference<jintArray>(), "");
static_assert(IsPlainJniReference<jbooleanArray>(), "");

static_assert(!IsPlainJniReference<local_ref<jobject>>(), "");
static_assert(!IsPlainJniReference<local_ref<sjf>>(), "");
static_assert(!IsPlainJniReference<global_ref<jobject>>(), "");
static_assert(!IsPlainJniReference<weak_ref<jobject>>(), "");
static_assert(!IsPlainJniReference<weak_ref<sjf>>(), "");
static_assert(!IsPlainJniReference<std::string>(), "");
static_assert(!IsPlainJniReference<jint*>(), "");
static_assert(!IsPlainJniReference<void>(), "");
static_assert(!IsPlainJniReference<int>(), "");


static_assert(IsJniPrimitive<int>(), "");
static_assert(IsJniPrimitive<jint>(), "");
static_assert(IsJniPrimitive<jboolean>(), "");

static_assert(!IsJniPrimitive<jobject>(), "");
static_assert(!IsJniPrimitive<sjf>(), "");
static_assert(!IsJniPrimitive<jintArray>(), "");
static_assert(!IsJniPrimitive<jbooleanArray>(), "");
static_assert(!IsJniPrimitive<local_ref<jobject>>(), "");
static_assert(!IsJniPrimitive<local_ref<sjf>>(), "");
static_assert(!IsJniPrimitive<global_ref<jobject>>(), "");
static_assert(!IsJniPrimitive<weak_ref<jobject>>(), "");
static_assert(!IsJniPrimitive<weak_ref<sjf>>(), "");
static_assert(!IsJniPrimitive<std::string>(), "");
static_assert(!IsJniPrimitive<jint*>(), "");
static_assert(!IsJniPrimitive<void>(), "");


static_assert(IsJniPrimitiveArray<jintArray>(), "");
static_assert(IsJniPrimitiveArray<jbooleanArray>(), "");

static_assert(!IsJniPrimitiveArray<int>(), "");
static_assert(!IsJniPrimitiveArray<jint>(), "");
static_assert(!IsJniPrimitiveArray<jboolean>(), "");
static_assert(!IsJniPrimitiveArray<jobject>(), "");
static_assert(!IsJniPrimitiveArray<sjf>(), "");
static_assert(!IsJniPrimitiveArray<local_ref<jobject>>(), "");
static_assert(!IsJniPrimitiveArray<local_ref<sjf>>(), "");
static_assert(!IsJniPrimitiveArray<global_ref<jobject>>(), "");
static_assert(!IsJniPrimitiveArray<weak_ref<jobject>>(), "");
static_assert(!IsJniPrimitiveArray<weak_ref<sjf>>(), "");
static_assert(!IsJniPrimitiveArray<std::string>(), "");
static_assert(!IsJniPrimitiveArray<jint*>(), "");
static_assert(!IsJniPrimitiveArray<void>(), "");


static_assert(IsJniScalar<jintArray>(), "");
static_assert(IsJniScalar<jbooleanArray>(), "");
static_assert(IsJniScalar<int>(), "");
static_assert(IsJniScalar<jint>(), "");
static_assert(IsJniScalar<jboolean>(), "");
static_assert(IsJniScalar<jobject>(), "");
static_assert(IsJniScalar<sjf>(), "");

static_assert(!IsJniScalar<local_ref<jobject>>(), "");
static_assert(!IsJniScalar<local_ref<sjf>>(), "");
static_assert(!IsJniScalar<global_ref<jobject>>(), "");
static_assert(!IsJniScalar<weak_ref<jobject>>(), "");
static_assert(!IsJniScalar<weak_ref<sjf>>(), "");
static_assert(!IsJniScalar<std::string>(), "");
static_assert(!IsJniScalar<jint*>(), "");
static_assert(!IsJniScalar<void>(), "");


static_assert(IsJniType<jintArray>(), "");
static_assert(IsJniType<jbooleanArray>(), "");
static_assert(IsJniType<int>(), "");
static_assert(IsJniType<jint>(), "");
static_assert(IsJniType<jboolean>(), "");
static_assert(IsJniType<jobject>(), "");
static_assert(IsJniType<sjf>(), "");
static_assert(IsJniType<void>(), "");

static_assert(!IsJniType<local_ref<jobject>>(), "");
static_assert(!IsJniType<local_ref<sjf>>(), "");
static_assert(!IsJniType<global_ref<jobject>>(), "");
static_assert(!IsJniType<weak_ref<jobject>>(), "");
static_assert(!IsJniType<weak_ref<sjf>>(), "");
static_assert(!IsJniType<std::string>(), "");
static_assert(!IsJniType<jint*>(), "");

constexpr const char* jaccess_class_name = "com/facebook/jni/FBJniTests";

struct _jfakeClass : _jobject {};
using jFakeClass = _jfakeClass*;

// This gives a better error message than doing the static_assert inline
// (because it will contain the two resolved types).
template <typename T, typename F>
void StaticAssertSame() {
  static_assert(std::is_same<T, F>::value, "");
};

} // namespace

void RegisterFbjniTests() {
  StaticAssertSame<JniType<jFakeClass>, jFakeClass>();

  StaticAssertSame<PrimitiveOrJniType<JObject>, jobject>();
  StaticAssertSame<PrimitiveOrJniType<JClass>, jclass>();
  StaticAssertSame<PrimitiveOrJniType<JArrayInt>, jintArray>();
  StaticAssertSame<PrimitiveOrJniType<jint>, jint>();
  StaticAssertSame<PrimitiveOrJniType<TestThing>, TestThing::javaobject>();

  registerNatives(jaccess_class_name, {
      makeNativeMethod("nativeTestClassResolution", TestClassResolution),
      makeNativeMethod("nativeTestLazyClassResolution", TestLazyClassResolution),
      makeNativeMethod("nativeCreateInstanceOf", TestCreateInstanceOf),
      makeNativeMethod("nativeTestVirtualMethodResolution_I", TestVirtualMethodResolution_I),
      makeNativeMethod("nativeTestTypeDescriptors", TestTypeDescriptors),
      makeNativeMethod(
          "nativeTestVirtualMethodResolution_arrB",
          TestVirtualMethodResolution_arrB),
      makeNativeMethod(
          "nativeTestVirtualMethodResolution_S_arrS",
          TestVirtualMethodResolution_S_arrS),
      makeNativeMethod(
          "nativeTestVirtualMethodResolution_arrarrS",
          TestVirtualMethodResolution_arrarrS),
      makeNativeMethod(
          "nativeTestVirtualMethodResolution_arrarrI",
          TestVirtualMethodResolution_arrarrI),
      makeNativeMethod(
          "nativeTestLazyVirtualMethodResolution_I",
          TestLazyVirtualMethodResolution_I),
      makeNativeMethod(
          "nativeTestJMethodCallbacks",
          "(Lcom/facebook/jni/FBJniTests$Callbacks;)V",
          TestJMethodCallbacks),
      makeNativeMethod("nativeTestJStaticMethodCallbacks", TestJStaticMethodCallbacks),
      makeNativeMethod("nativeTestIsAssignableFrom", TestIsAssignableFrom),
      makeNativeMethod("nativeTestIsInstanceOf", TestIsInstanceOf),
      makeNativeMethod("nativeTestIsSameObject", TestIsSameObject),
      makeNativeMethod("nativeTestGetSuperclass", TestGetSuperclass),
      makeNativeMethod("nativeTestWeakRefs", TestWeakRefs),
      makeNativeMethod("nativeTestAlias", TestAlias),
      makeNativeMethod("nativeTestAutoAliasRefReturningVoid", TestAutoAliasRefReturningVoid),
      makeNativeMethod("nativeTestAliasRefConversions", testAliasRefConversions),
      makeNativeMethod("nativeTestCreatingReferences", testCreatingReferences),
      makeNativeMethod(
          "nativeTestAssignmentAndCopyConstructors",
          testAssignmentAndCopyConstructors),
      makeNativeMethod(
          "nativeTestAssignmentAndCopyCrossTypes",
          testAssignmentAndCopyCrossTypes),
      makeNativeMethod("nativeTestNullReferences", testNullReferences),
      makeNativeMethod("nativeTestFieldAccess", TestFieldAccess),
      makeNativeMethod("nativeTestStringFieldAccess", TestStringFieldAccess),
      makeNativeMethod("nativeTestReferenceFieldAccess", TestReferenceFieldAccess),
      makeNativeMethod("nativeTestStaticFieldAccess", TestStaticFieldAccess),
      makeNativeMethod("nativeTestStaticStringFieldAccess", TestStaticStringFieldAccess),
      makeNativeMethod("nativeTestStaticReferenceFieldAccess", TestStaticReferenceFieldAccess),
      makeNativeMethod("nativeTestNonVirtualMethod", TestNonVirtualMethod),
      makeNativeMethod("nativeTestArrayCreation", TestArrayCreation),
      makeNativeMethod("nativeTestMultidimensionalObjectArray", TestMultidimensionalObjectArray),
      makeNativeMethod("nativeTestMultidimensionalPrimitiveArray", TestMultidimensionalPrimitiveArray),
      makeNativeMethod("nativeTestBuildStringArray", TestBuildStringArray),
      makeNativeMethod("testHandleJavaCustomExceptionNative", TestHandleJavaCustomException),
      makeNativeMethod("testHandleNullExceptionMessageNative", TestHandleNullExceptionMessage),
      makeNativeMethod("nativeTestHandleNestedException", TestHandleNestedException),
      makeNativeMethod("nativeTestHandleNoRttiException", TestHandleNoRttiException),
      makeNativeMethod("nativeTestCopyConstructor", TestCopyConstructor),
      makeNativeMethod(
          "nativeTestMoveConstructorWithEmptyWhat",
          TestMoveConstructorWithEmptyWhat),
      makeNativeMethod(
          "nativeTestMoveConstructorWithPopulatedWhat",
          TestMoveConstructorWithPopulatedWhat),
      makeNativeMethod("nativeTestHandleCppRuntimeError", TestHandleCppRuntimeError),
      makeNativeMethod("nativeTestHandleCppIOBaseFailure", TestHandleCppIOBaseFailure),
      makeNativeMethod("nativeTestHandleCppSystemError", TestHandleCppSystemError),
      makeNativeMethod("nativeTestInterDsoExceptionHandlingA", TestInterDsoExceptionHandlingA),
      makeNativeMethod("nativeTestInterDsoExceptionHandlingB", TestInterDsoExceptionHandlingB),
      makeNativeMethod("nativeTestHandleNonStdExceptionThrow", TestHandleNonStdException),
      makeNativeMethod("nativeTestHandleCppIntThrow", TestHandleCppIntThrow),
      makeNativeMethod("nativeTestHandleCppCharPointerThrow", TestHandleCppCharPointerThrow),
      makeNativeMethod("nativeTestThrowJavaExceptionByName", TestThrowJavaExceptionByName),
      makeNativeMethod("nativeTestJThread", TestJThread),
      makeNativeMethod("nativeTestThreadScopeGuard", TestThreadScopeGuard),
      makeNativeMethod("nativeTestNestedThreadScopeGuard", TestNestedThreadScopeGuard),
      makeNativeMethod("nativeTestClassLoadInWorker", TestClassLoadInWorker),
      makeNativeMethod("nativeTestClassLoadWorkerFastPath", TestClassLoadWorkerFastPath),
      makeNativeMethod("nativeTestHandleCppCharPointerThrow", TestHandleCppCharPointerThrow),
      makeNativeMethod("nativeTestToString", testToString),
      makeNativeMethod("nativeTestThrowJavaExceptionByName", TestThrowJavaExceptionByName),
      makeNativeMethod("nativeTestThreadScopeGuard", TestThreadScopeGuard),
      makeNativeMethod("nativeTestNestedThreadScopeGuard", TestNestedThreadScopeGuard),
      makeNativeMethod("nativeTestNullJString", testNullJString),
      makeNativeMethod("nativeTestSwap", testSwap),
      makeNativeMethod("nativeTestEqualOperator", testEqualOperator),
      makeNativeMethod("nativeTestReleaseAlias", testReleaseAlias),
      makeNativeMethod("nativeTestLockingWeakReferences", testLockingWeakReferences),
      makeNativeMethod("nativeStaticCastAliasRefToString", StaticCastAliasRefToString),
      makeNativeMethod("nativeDynamicCastAliasRefToThrowable", DynamicCastAliasRefToThrowable),
      makeNativeMethod("nativeStaticCastLocalRefToString", StaticCastLocalRefToString),
      makeNativeMethod("nativeDynamicCastLocalRefToString", DynamicCastLocalRefToString),
      makeNativeMethod("nativeStaticCastGlobalRefToString", StaticCastGlobalRefToString),
      makeNativeMethod("nativeDynamicCastGlobalRefToString", DynamicCastGlobalRefToString),
      makeNativeMethod("testMethodResolutionWithCxxTypesNative", TestMethodResolutionWithCxxTypes),
      makeCriticalNativeMethod_DO_NOT_USE_OR_YOU_WILL_BE_FIRED("nativeCriticalNativeMethodBindsAndCanBeInvoked", testCriticalNativeMethodBindsAndCanBeInvoked),
  });
}
