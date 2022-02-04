/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_JAVA

#include <fbjni/fbjni.h>

namespace hermes {
namespace platform_unicode {

using namespace facebook::jni;

namespace {

void copyStringTo(
    JNIEnv *env,
    local_ref<jstring> jstr,
    llvh::SmallVectorImpl<char16_t> &buf) {
  jsize len = env->GetStringLength(jstr.get());
  const jchar *chr = env->GetStringChars(jstr.get(), nullptr);
  buf.assign(chr, chr + len);
  env->ReleaseStringChars(jstr.get(), chr);
}

/// Converts a SmallVector of char16_t into a Java \c String.
/// \p env The JNI environment to create the string in.
/// \p str The string to copy into the Java Heap.
/// \return the resulting string.
local_ref<JString> toJavaString(JNIEnv *env, llvh::ArrayRef<char16_t> str) {
  static_assert(
      sizeof(jchar) == sizeof(*str.data()),
      "UTF16 char not the same size as Java char.");

  auto jStr = adopt_local(
      env->NewString(reinterpret_cast<const jchar *>(str.data()), str.size()));

  FACEBOOK_JNI_THROW_PENDING_EXCEPTION();

  return jStr;
}

} // anonymous namespace

/// Provides JNI bindings for using existing Android APIs to perform
/// unicode-related tasks.
/// NOTE: All public exports from this header should be marked `noexcept`.
class JAndroidUnicodeUtils
    : public facebook::jni::JavaClass<JAndroidUnicodeUtils> {
  /// Abort if there exists a pending Java exception. This throws the Java
  /// exception out of a noexcept handler, which will call std::terminate().
  static void abortOnJavaException() noexcept {
    FACEBOOK_JNI_THROW_PENDING_EXCEPTION();
  }

 public:
  static auto constexpr kJavaDescriptor =
      "Lcom/facebook/hermes/unicode/AndroidUnicodeUtils;";

  /// Compares two strings using the user's chosen locale to order chars.
  /// Equivalent of left.localeCompare(right) in JS.
  static jint localeCompare(
      llvh::ArrayRef<char16_t> left,
      llvh::ArrayRef<char16_t> right) noexcept {
    const auto env = facebook::jni::Environment::current();
    auto jLeft = toJavaString(env, left);
    auto jRight = toJavaString(env, right);

    static const auto jLocaleCompare =
        javaClassStatic()->getStaticMethod<jint(jstring, jstring)>(
            "localeCompare");

    return jLocaleCompare(javaClassStatic(), jLeft.get(), jRight.get());
  }

  static void dateFormat(
      double unixtimeMs,
      bool formatDate,
      bool formatTime,
      llvh::SmallVectorImpl<char16_t> &buf) noexcept {
    const auto env = facebook::jni::Environment::current();
    static const auto jDateFormat =
        javaClassStatic()
            ->getStaticMethod<jstring(jdouble, jboolean, jboolean)>(
                "dateFormat");
    local_ref<jstring> javaFormattedDate =
        jDateFormat(javaClassStatic(), unixtimeMs, formatDate, formatTime);

    FACEBOOK_JNI_THROW_PENDING_EXCEPTION();

    jsize len = env->GetStringLength(javaFormattedDate.get());
    const jchar *chr = env->GetStringChars(javaFormattedDate.get(), nullptr);
    buf.append(chr, chr + len);
    env->ReleaseStringChars(javaFormattedDate.get(), chr);
  }

  static void convertToCase(
      llvh::SmallVectorImpl<char16_t> &buf,
      CaseConversion targetCase,
      bool useCurrentLocale) noexcept {
    const auto env = facebook::jni::Environment::current();
    static const auto jConvertCase =
        javaClassStatic()->getStaticMethod<jstring(jstring, int, jboolean)>(
            "convertToCase");
    auto jInput = toJavaString(env, buf);
    int targetCaseInt = static_cast<int>(targetCase);
    local_ref<jstring> javaConvertedCase = jConvertCase(
        javaClassStatic(), jInput.get(), targetCaseInt, useCurrentLocale);

    abortOnJavaException();
    copyStringTo(env, javaConvertedCase, buf);
  }

  static void normalize(
      llvh::SmallVectorImpl<char16_t> &buf,
      NormalizationForm form) noexcept {
    const auto env = facebook::jni::Environment::current();
    static const auto jNormalize =
        javaClassStatic()->getStaticMethod<jstring(jstring, int)>("normalize");
    auto jInput = toJavaString(env, buf);
    int formInt = static_cast<int>(form);
    local_ref<jstring> javaNormalized =
        jNormalize(javaClassStatic(), jInput.get(), formInt);

    abortOnJavaException();
    copyStringTo(env, javaNormalized, buf);
  }
};

int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  return JAndroidUnicodeUtils::localeCompare(left, right);
}

void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf) {
  JAndroidUnicodeUtils::dateFormat(unixtimeMs, formatDate, formatTime, buf);
}

void convertToCase(
    llvh::SmallVectorImpl<char16_t> &str,
    CaseConversion targetCase,
    bool useCurrentLocale) {
  JAndroidUnicodeUtils::convertToCase(str, targetCase, useCurrentLocale);
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  JAndroidUnicodeUtils::normalize(buf, form);
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_JAVA
