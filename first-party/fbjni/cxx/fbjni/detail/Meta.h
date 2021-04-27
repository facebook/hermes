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

/** @file meta.h
 *
 * Provides wrappers for meta data such as methods and fields.
 */

#pragma once

#include <type_traits>
#include <string>

#include <jni.h>

#include <fbjni/detail/FbjniApi.h>
#include <fbjni/detail/SimpleFixedString.h>
#include "References-forward.h"

#ifdef __ANDROID__
# include <android/log.h>
# define XLOG_TAG "fb-jni"
# define XLOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, XLOG_TAG, __VA_ARGS__)
# define XLOGD(...) __android_log_print(ANDROID_LOG_DEBUG, XLOG_TAG, __VA_ARGS__)
# define XLOGI(...) __android_log_print(ANDROID_LOG_INFO, XLOG_TAG, __VA_ARGS__)
# define XLOGW(...) __android_log_print(ANDROID_LOG_WARN, XLOG_TAG, __VA_ARGS__)
# define XLOGE(...) __android_log_print(ANDROID_LOG_ERROR, XLOG_TAG, __VA_ARGS__)
# define XLOGWTF(...) __android_log_print(ANDROID_LOG_FATAL, XLOG_TAG, __VA_ARGS__)
#endif

namespace facebook {
namespace jni {

// This will get the reflected Java Method from the method_id, get it's invoke
// method, and call the method via that. This shouldn't ever be needed, but
// Android 6.0 crashes when calling a method on a java.lang.Proxy via jni.
template <typename... Args>
local_ref<jobject> slowCall(jmethodID method_id, alias_ref<jobject> self, Args... args);

class JObject;


/// Wrapper of a jmethodID. Provides a common base for JMethod specializations
class JMethodBase {
 public:
  /// Verify that the method is valid
  explicit operator bool() const noexcept;

  /// Access the wrapped id
  jmethodID getId() const noexcept;

 protected:
  /// Create a wrapper of a method id
  explicit JMethodBase(jmethodID method_id = nullptr) noexcept;

 private:
  jmethodID method_id_;
};


/// Representation of a jmethodID
template<typename F>
class JMethod;

/// @cond INTERNAL
#pragma push_macro("DEFINE_PRIMITIVE_METHOD_CLASS")

#undef DEFINE_PRIMITIVE_METHOD_CLASS

// Defining JMethod specializations based on return value
#define DEFINE_PRIMITIVE_METHOD_CLASS(TYPE)                                      \
template<typename... Args>                                                       \
class JMethod<TYPE(Args...)> : public JMethodBase {                              \
 public:                                                                         \
  static_assert(std::is_void<TYPE>::value || IsJniPrimitive<TYPE>(),             \
      "TYPE must be primitive or void");                                         \
                                                                                 \
  using JMethodBase::JMethodBase;                                                \
  JMethod() noexcept {};                                                         \
  JMethod(const JMethod& other) noexcept = default;                              \
                                                                                 \
  TYPE operator()(alias_ref<jobject> self, Args... args) const;                  \
                                                                                 \
  friend class JClass;                                                           \
}

DEFINE_PRIMITIVE_METHOD_CLASS(void);
DEFINE_PRIMITIVE_METHOD_CLASS(jboolean);
DEFINE_PRIMITIVE_METHOD_CLASS(jbyte);
DEFINE_PRIMITIVE_METHOD_CLASS(jchar);
DEFINE_PRIMITIVE_METHOD_CLASS(jshort);
DEFINE_PRIMITIVE_METHOD_CLASS(jint);
DEFINE_PRIMITIVE_METHOD_CLASS(jlong);
DEFINE_PRIMITIVE_METHOD_CLASS(jfloat);
DEFINE_PRIMITIVE_METHOD_CLASS(jdouble);

#pragma pop_macro("DEFINE_PRIMITIVE_METHOD_CLASS")
/// @endcond


/// Convenience type representing constructors
/// These should only be used with JClass::getConstructor and JClass::newObject.
template<typename F>
struct JConstructor : private JMethod<F> {
  using JMethod<F>::JMethod;
 private:
  JConstructor(const JMethod<F>& other) : JMethod<F>(other.getId()) {}
  friend class JClass;
};

/// Representation of a jStaticMethodID
template<typename F>
class JStaticMethod;

/// @cond INTERNAL
#pragma push_macro("DEFINE_PRIMITIVE_STATIC_METHOD_CLASS")

#undef DEFINE_PRIMITIVE_STATIC_METHOD_CLASS

// Defining JStaticMethod specializations based on return value
#define DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(TYPE)                          \
template<typename... Args>                                                  \
class JStaticMethod<TYPE(Args...)> : public JMethodBase {                   \
  static_assert(std::is_void<TYPE>::value || IsJniPrimitive<TYPE>(),        \
      "T must be a JNI primitive or void");                                 \
                                                                            \
 public:                                                                    \
  using JMethodBase::JMethodBase;                                           \
  JStaticMethod() noexcept {};                                              \
  JStaticMethod(const JStaticMethod& other) noexcept = default;             \
                                                                            \
  TYPE operator()(alias_ref<jclass> cls, Args... args) const;               \
                                                                            \
  friend class JClass;                                                      \
}

DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(void);
DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(jboolean);
DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(jbyte);
DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(jchar);
DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(jshort);
DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(jint);
DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(jlong);
DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(jfloat);
DEFINE_PRIMITIVE_STATIC_METHOD_CLASS(jdouble);

#pragma pop_macro("DEFINE_PRIMITIVE_STATIC_METHOD_CLASS")
/// @endcond


/// Representation of a jNonvirtualMethodID
template<typename F>
class JNonvirtualMethod;

/// @cond INTERNAL
#pragma push_macro("DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS")

#undef DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS

// Defining JNonvirtualMethod specializations based on return value
#define DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(TYPE)                     \
template<typename... Args>                                                  \
class JNonvirtualMethod<TYPE(Args...)> : public JMethodBase {               \
  static_assert(std::is_void<TYPE>::value || IsJniPrimitive<TYPE>(),        \
      "T must be a JNI primitive or void");                                 \
                                                                            \
 public:                                                                    \
  using JMethodBase::JMethodBase;                                           \
  JNonvirtualMethod() noexcept {};                                          \
  JNonvirtualMethod(const JNonvirtualMethod& other) noexcept = default;     \
                                                                            \
  TYPE operator()(alias_ref<jobject> self, alias_ref<jclass> cls, Args... args) const; \
                                                                            \
  friend class JClass;                                                      \
}

DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(void);
DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(jboolean);
DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(jbyte);
DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(jchar);
DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(jshort);
DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(jint);
DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(jlong);
DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(jfloat);
DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS(jdouble);

#pragma pop_macro("DEFINE_PRIMITIVE_NON_VIRTUAL_METHOD_CLASS")
/// @endcond


/**
 * JField represents typed fields and simplifies their access. Note that object types return
 * raw pointers which generally should promptly get a wrap_local treatment.
 */
template<typename T>
class JField {
  static_assert(IsJniScalar<T>(), "T must be a JNI scalar");

 public:
  /// Wraps an existing field id
  explicit JField(jfieldID field = nullptr) noexcept;

  /// Verify that the id is valid
  explicit operator bool() const noexcept;

  /// Access the wrapped id
  jfieldID getId() const noexcept;

 private:
  jfieldID field_id_;

  /// Get field value
  /// @pre object != nullptr
  T get(jobject object) const noexcept;

  /// Set field value
  /// @pre object != nullptr
  void set(jobject object, T value) noexcept;

  friend class JObject;
};


/**
 * JStaticField represents typed fields and simplifies their access. Note that object types
 * return raw pointers which generally should promptly get a wrap_local treatment.
 */
template<typename T>
class JStaticField {
  static_assert(IsJniScalar<T>(), "T must be a JNI scalar");

 public:
  /// Wraps an existing field id
  explicit JStaticField(jfieldID field = nullptr) noexcept;

  /// Verify that the id is valid
  explicit operator bool() const noexcept;

  /// Access the wrapped id
  jfieldID getId() const noexcept;

 private:
  jfieldID field_id_;

  /// Get field value
  /// @pre object != nullptr
  T get(jclass jcls) const noexcept;

  /// Set field value
  /// @pre object != nullptr
  void set(jclass jcls, T value) noexcept;

  friend class JClass;
  friend class JObject;
};


// jtype_traits ////////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct jtype_traits {
private:
  using Repr = ReprType<T>;
  static constexpr auto /* detail::SimpleFixedString<_> */ descriptor() {
    constexpr auto len = Repr::kJavaDescriptor
      ? detail::constexpr_strlen(Repr::kJavaDescriptor)
      : Repr::get_instantiated_java_descriptor().size();
    if (Repr::kJavaDescriptor) {
      return detail::SimpleFixedString<len>(Repr::kJavaDescriptor, len);
    } else {
      return detail::SimpleFixedString<len>(Repr::get_instantiated_java_descriptor());
    }
  }
  static constexpr auto /* detail::SimpleFixedString<_> */ base_name() {
    constexpr auto len = Repr::kJavaDescriptor ? detail::constexpr_strlen(Repr::kJavaDescriptor) - 2 : Repr::get_instantiated_base_name().size();
    if (Repr::kJavaDescriptor) {
      detail::SimpleFixedString<len + 2> result(Repr::kJavaDescriptor, len + 2);
      return detail::SimpleFixedString<len>(result.substr(1, result.size() - 2));
    }
    return detail::SimpleFixedString<len>(Repr::get_instantiated_base_name());
  }
 public:
  using descriptorType = decltype(jtype_traits<T>::descriptor());
  using basenameType = decltype(jtype_traits<T>::base_name());

  // The jni type signature (described at
  // http://docs.oracle.com/javase/1.5.0/docs/guide/jni/spec/types.html).
  static constexpr descriptorType /* detail::SimpleFixedString<_> */ kDescriptor = descriptor();

  // The signature used for class lookups. See
  // http://docs.oracle.com/javase/6/docs/api/java/lang/Class.html#getName().
  static constexpr basenameType /* detail::SimpleFixedString<_> */ kBaseName = base_name();
};

template <typename T>
constexpr typename jtype_traits<T>::descriptorType jtype_traits<T>::kDescriptor;
template <typename T>
constexpr typename jtype_traits<T>::basenameType jtype_traits<T>::kBaseName;

static_assert(
  std::is_same<jint, int>::value,
  "jint must be int.  On Windows, try using Android's jni.h.");

#pragma push_macro("DEFINE_FIELD_AND_ARRAY_TRAIT")
#undef DEFINE_FIELD_AND_ARRAY_TRAIT

// NOTE: When updating this definition, see also DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT in Meta.cpp.
#define DEFINE_FIELD_AND_ARRAY_TRAIT(TYPE, DSC)                         \
  template<>                                                            \
struct FBJNI_API jtype_traits<TYPE> {                                             \
  static constexpr decltype(detail::makeSimpleFixedString(#DSC)) kDescriptor = detail::makeSimpleFixedString(#DSC); \
  static constexpr decltype(kDescriptor) kBaseName = kDescriptor;       \
  using array_type = TYPE ## Array;                                     \
};                                                                      \
                                                                        \
template<>                                                              \
struct FBJNI_API jtype_traits<TYPE ## Array> {                                    \
  static constexpr decltype(detail::makeSimpleFixedString("[" #DSC)) kDescriptor = detail::makeSimpleFixedString("[" #DSC); \
  static constexpr decltype(jtype_traits<TYPE ## Array>::kDescriptor) kBaseName = kDescriptor; \
  using entry_type = TYPE;                                              \
};


// There is no voidArray, handle that without the macro.
template<>
struct FBJNI_API jtype_traits<void> {
  static constexpr detail::SimpleFixedString<1> kDescriptor = detail::makeSimpleFixedString("V");
};

DEFINE_FIELD_AND_ARRAY_TRAIT(jboolean, Z)
DEFINE_FIELD_AND_ARRAY_TRAIT(jbyte,    B)
DEFINE_FIELD_AND_ARRAY_TRAIT(jchar,    C)
DEFINE_FIELD_AND_ARRAY_TRAIT(jshort,   S)
DEFINE_FIELD_AND_ARRAY_TRAIT(jint,     I)
DEFINE_FIELD_AND_ARRAY_TRAIT(jlong,    J)
DEFINE_FIELD_AND_ARRAY_TRAIT(jfloat,   F)
DEFINE_FIELD_AND_ARRAY_TRAIT(jdouble,  D)

#pragma pop_macro("DEFINE_FIELD_AND_ARRAY_TRAIT")


template <typename T>
struct jmethod_traits_from_cxx;

}}

#include "Meta-inl.h"

namespace facebook {
namespace jni {
/// Template magic to provide @ref jmethod_traits
template<typename R, typename... Args>
struct jmethod_traits<R(Args...)> {
  static constexpr decltype(internal::JMethodDescriptor<R, Args...>()) /* detail::SimpleFixedString */ kDescriptor = internal::JMethodDescriptor<R, Args...>();
  static constexpr decltype(internal::JMethodDescriptor<void, Args...>()) /* detail::SimpleFixedString */ kConstructorDescriptor = internal::JMethodDescriptor<void, Args...>();
};

template<typename R, typename...Args>
/*static*/ constexpr decltype(internal::JMethodDescriptor<R, Args...>()) /* detail::SimpleFixedString */ jmethod_traits<R(Args...)>::kDescriptor;

template<typename R, typename...Args>
/*static*/ constexpr decltype(internal::JMethodDescriptor<void, Args...>()) /* detail::SimpleFixedString */ jmethod_traits<R(Args...)>::kConstructorDescriptor;

}}
