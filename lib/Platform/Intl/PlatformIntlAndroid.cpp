/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/PlatformIntl.h"

// Android ICU uses different package names than ICU4J, and claims
// other differences.  So for now, consider this impl specific to
// Android.  It's likely it could be made to work against the
// non-Android ICU4J packages, too, if necessary, but it would take a
// bit more work.

#include <fbjni/fbjni.h>

using namespace ::facebook;
using namespace ::hermes;

namespace hermes {
namespace platform_intl {

namespace {
/// Traits class used by the IntlBridge template below.
template <typename IntlType>
struct IntlBridgeTraits;

/// Connects a native android intl object with a Hermes Intl API. The bridge has
/// a single method named "native" which can then be used to invoke the native
/// platform APIs.
template <typename IntlType>
class IntlBridge : public IntlType {
 public:
  using NativeType = typename IntlBridgeTraits<IntlType>::NativeType;

  template <typename... Args>
  explicit IntlBridge(Args &&...args)
      : native_(
            jni::make_global(NativeType::create(std::forward<Args>(args)...))) {
  }

  ~IntlBridge() {
    jni::ThreadScope::WithClassLoader([&] { native_.reset(); });
  }

  jni::global_ref<NativeType> &native() {
    return native_;
  }

 private:
  jni::global_ref<NativeType> native_;
};

/// Helper function that extracts the "native" payload off of an IntlBridge
/// object.
template <typename IntlType>
jni::global_ref<typename IntlBridge<IntlType>::NativeType> &native(
    IntlType *self) {
  return static_cast<IntlBridge<IntlType> *>(self)->native();
}

template <typename E = jobject>
struct JArrayList : jni::JavaClass<JArrayList<E>, jni::JList<E>> {
  constexpr static auto kJavaDescriptor = "Ljava/util/ArrayList;";

  using Super = jni::JavaClass<JArrayList<E>, jni::JList<E>>;

  static jni::local_ref<JArrayList<E>> create() {
    return Super::newInstance();
  }

  static jni::local_ref<JArrayList<E>> create(int initialCapacity) {
    return Super::newInstance(initialCapacity);
  }

  bool add(jni::alias_ref<jobject> elem) {
    static auto addMethod =
        Super::javaClassStatic()
            ->template getMethod<jboolean(jni::alias_ref<jobject>)>("add");
    return addMethod(Super::self(), elem);
  }
};

template <typename K = jobject, typename V = jobject>
struct JHashMap : jni::JavaClass<JHashMap<K, V>, jni::JMap<K, V>> {
  constexpr static auto kJavaDescriptor = "Ljava/util/HashMap;";

  using Super = jni::JavaClass<JHashMap<K, V>, jni::JMap<K, V>>;

  static jni::local_ref<JHashMap<K, V>> create() {
    return Super::newInstance();
  }

  void put(jni::alias_ref<jobject> key, jni::alias_ref<jobject> val) {
    static auto putMethod =
        Super::javaClassStatic()
            ->template getMethod<jni::alias_ref<jobject>(
                jni::alias_ref<jobject>, jni::alias_ref<jobject>)>("put");
    putMethod(Super::self(), key, val);
  }
};

using JLocalesList = jni::JList<jni::JString>;
using JOptionsMap = jni::JMap<jni::JString, jni::JObject>;
using JPartMap = jni::JMap<jni::JString, jni::JString>;
using JPartsList = jni::JList<JPartMap>;

jni::local_ref<jstring> stringToJava(const std::u16string &utf16) {
  // Work around a bug in fbjni where make_jstring returns null for empty
  // u16strings.
  // TODO(T101910387): Switch back to make_jstring once it is fixed.
  const auto env = jni::Environment::current();
  static_assert(
      sizeof(jchar) == sizeof(std::u16string::value_type),
      "Expecting jchar to be the same size as std::u16string::CharT");
  jstring result = env->NewString(
      reinterpret_cast<const jchar *>(utf16.c_str()), utf16.size());
  FACEBOOK_JNI_THROW_PENDING_EXCEPTION();
  return jni::adopt_local(result);
}

jni::local_ref<JLocalesList> localesToJava(
    std::vector<std::u16string> locales) {
  jni::local_ref<JArrayList<jni::JString>> ret =
      JArrayList<jni::JString>::create(locales.size());
  for (const auto &locale : locales) {
    ret->add(stringToJava(locale));
  }
  return ret;
}

jni::local_ref<JOptionsMap> optionsToJava(const Options &options) {
  auto ret = JHashMap<jni::JString, jni::JObject>::create();
  for (const auto &kv : options) {
    jni::local_ref<jni::JObject> jvalue;
    if (kv.second.isBool()) {
      jvalue = jni::autobox(static_cast<jboolean>(kv.second.getBool()));
    } else if (kv.second.isNumber()) {
      jvalue = jni::autobox(static_cast<jdouble>(kv.second.getNumber()));
    } else {
      assert(kv.second.isString() && "Option is not valid type");
      jvalue = stringToJava(kv.second.getString());
    }
    ret->put(stringToJava(kv.first), jvalue);
  }
  return ret;
}

std::u16string stringFromJava(jni::alias_ref<jni::JString> result) {
  return result->toU16String();
}

Options optionsFromJava(jni::alias_ref<JOptionsMap> result) {
  if (!result) {
    return Options();
  }

  Options ret;
  for (const auto &kv : *result) {
    if (!kv.first || !kv.second) {
      // ignore nulls
      continue;
    }

    if (jni::JBoolean::javaClassStatic()->isAssignableFrom(
            kv.second->getClass())) {
      ret.emplace(
          stringFromJava(kv.first),
          Option(static_cast<bool>(
              jni::static_ref_cast<jni::JBoolean>(kv.second)->booleanValue())));
    } else if (jni::JInteger::javaClassStatic()->isAssignableFrom(
                   kv.second->getClass())) {
      ret.emplace(
          stringFromJava(kv.first),
          Option(static_cast<double>(
              jni::static_ref_cast<jni::JInteger>(kv.second)->intValue())));
    } else if (jni::JDouble::javaClassStatic()->isAssignableFrom(
                   kv.second->getClass())) {
      ret.emplace(
          stringFromJava(kv.first),
          Option(jni::static_ref_cast<jni::JDouble>(kv.second)->doubleValue()));
    } else if (jni::JString::javaClassStatic()->isAssignableFrom(
                   kv.second->getClass())) {
      ret.emplace(
          stringFromJava(kv.first),
          Option(
              stringFromJava(jni::static_ref_cast<jni::JString>(kv.second))));
    } else {
      // ignore mistyped value
    }
  }
  return ret;
}

// Part: Map<String, String>
Part partFromJava(jni::alias_ref<JPartMap> result) {
  if (!result) {
    return Part();
  }

  Part ret;
  for (const auto &kv : *result) {
    if (!kv.first || !kv.second) {
      // ignore nulls
      continue;
    }

    ret.emplace(stringFromJava(kv.first), stringFromJava(kv.second));
  }
  return ret;
}

vm::CallResult<std::vector<std::u16string>> localesFromJava(
    vm::Runtime &runtime,
    vm::CallResult<jni::local_ref<JLocalesList>> &&result) {
  if (LLVM_UNLIKELY(result == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  std::vector<std::u16string> ret;
  if (!*result) {
    return std::vector<std::u16string>();
  }

  for (const auto &element : **result) {
    ret.push_back(stringFromJava(element));
  }
  return ret;
}

std::vector<Part> partsFromJava(jni::local_ref<JPartsList> &&result) {
  std::vector<Part> ret;
  if (!result) {
    return {};
  }

  for (const auto &element : *result) {
    ret.push_back(partFromJava(element));
  }
  return ret;
}

class JIntl : public jni::JavaClass<JIntl> {
 public:
  static constexpr auto kJavaDescriptor = "Lcom/facebook/hermes/intl/Intl;";

  static jni::local_ref<JLocalesList> getCanonicalLocales(
      jni::alias_ref<JLocalesList> locales) {
    static const auto method =
        javaClassStatic()
            ->getStaticMethod<jni::local_ref<JLocalesList>(
                jni::alias_ref<JLocalesList> locales)>("getCanonicalLocales");
    return method(javaClassStatic(), locales);
  }

  static jni::local_ref<jstring> toLocaleLowerCase(
      jni::alias_ref<JLocalesList> locales,
      jni::alias_ref<jstring> str) {
    auto method =
        javaClassStatic()
            ->getStaticMethod<jni::local_ref<jstring>(
                jni::alias_ref<JLocalesList> locales, jni::alias_ref<jstring>)>(
                "toLocaleLowerCase");
    return method(javaClassStatic(), locales, str);
  }

  static jni::local_ref<jstring> toLocaleUpperCase(
      jni::alias_ref<JLocalesList> locales,
      jni::alias_ref<jstring> str) {
    static const auto method =
        javaClassStatic()
            ->getStaticMethod<jni::local_ref<jstring>(
                jni::alias_ref<JLocalesList> locales, jni::alias_ref<jstring>)>(
                "toLocaleUpperCase");
    return method(javaClassStatic(), locales, str);
  }
};

} // namespace

vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales) {
  try {
    return localesFromJava(
        runtime, JIntl::getCanonicalLocales(localesToJava(locales)));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  try {
    return stringFromJava(
        JIntl::toLocaleLowerCase(localesToJava(locales), stringToJava(str)));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  try {
    return stringFromJava(
        JIntl::toLocaleUpperCase(localesToJava(locales), stringToJava(str)));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

namespace {

class JCollator : public jni::JavaClass<JCollator> {
 public:
  static constexpr auto kJavaDescriptor = "Lcom/facebook/hermes/intl/Collator;";

  static jni::local_ref<javaobject> create(
      jni::alias_ref<JLocalesList> locales,
      jni::alias_ref<JOptionsMap> options) {
    return newInstance(locales, options);
  }

  static jni::local_ref<JLocalesList> supportedLocalesOf(
      jni::alias_ref<JLocalesList> locales,
      jni::alias_ref<JOptionsMap> options) {
    static const auto method =
        javaClassStatic()
            ->getStaticMethod<jni::local_ref<JLocalesList>(
                jni::alias_ref<JLocalesList> locales,
                jni::alias_ref<JOptionsMap> options)>("supportedLocalesOf");
    return method(javaClassStatic(), locales, options);
  }

  jni::local_ref<JOptionsMap> resolvedOptions() {
    static const auto method =
        javaClassStatic()->getMethod<jni::local_ref<JOptionsMap>()>(
            "resolvedOptions");
    return method(self());
  }

  double compare(jni::alias_ref<jstring> x, jni::alias_ref<jstring> y) {
    static const auto method =
        javaClassStatic()
            ->getMethod<double(
                jni::alias_ref<jstring>, jni::alias_ref<jstring>)>("compare");
    return method(self(), x, y);
  }
};

} // namespace

Collator::Collator() = default;

Collator::~Collator() = default;

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return localesFromJava(
        runtime,
        JCollator::supportedLocalesOf(
            localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

template <>
struct IntlBridgeTraits<Collator> {
  using NativeType = JCollator;
};

vm::CallResult<std::unique_ptr<Collator>> Collator::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return std::make_unique<IntlBridge<Collator>>(
        localesToJava(locales), optionsToJava(options));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

Options Collator::resolvedOptions() noexcept {
  return optionsFromJava(native(this)->resolvedOptions());
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return native(this)->compare(stringToJava(x), stringToJava(y));
}

namespace {

class JDateTimeFormat : public jni::JavaClass<JDateTimeFormat> {
 public:
  static constexpr auto kJavaDescriptor =
      "Lcom/facebook/hermes/intl/DateTimeFormat;";

  static jni::local_ref<javaobject> create(
      jni::alias_ref<JLocalesList> locales,
      jni::alias_ref<JOptionsMap> options) {
    return newInstance(locales, options);
  }

  static jni::local_ref<JLocalesList> supportedLocalesOf(
      jni::alias_ref<JLocalesList> locales,
      jni::alias_ref<JOptionsMap> options) {
    static const auto method =
        javaClassStatic()
            ->getStaticMethod<jni::local_ref<JLocalesList>(
                jni::alias_ref<JLocalesList> locales,
                jni::alias_ref<JOptionsMap> options)>("supportedLocalesOf");
    return method(javaClassStatic(), locales, options);
  }

  jni::local_ref<JOptionsMap> resolvedOptions() {
    static const auto method =
        javaClassStatic()->getMethod<jni::local_ref<JOptionsMap>()>(
            "resolvedOptions");
    return method(self());
  }

  jni::local_ref<jstring> format(double jsTimeValue) {
    static const auto method =
        javaClassStatic()->getMethod<jni::alias_ref<jstring>(double)>("format");
    return method(self(), jsTimeValue);
  }

  jni::local_ref<JPartsList> formatToParts(double jsTimeValue) {
    static const auto method =
        javaClassStatic()->getMethod<jni::alias_ref<JPartsList>(double)>(
            "formatToParts");
    return method(self(), jsTimeValue);
  }
};

} // namespace

DateTimeFormat::DateTimeFormat() = default;

DateTimeFormat::~DateTimeFormat() = default;

vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return localesFromJava(
        runtime,
        JDateTimeFormat::supportedLocalesOf(
            localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

template <>
struct IntlBridgeTraits<DateTimeFormat> {
  using NativeType = JDateTimeFormat;
};

vm::CallResult<std::unique_ptr<DateTimeFormat>> DateTimeFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return std::make_unique<IntlBridge<DateTimeFormat>>(
        localesToJava(locales), optionsToJava(options));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

Options DateTimeFormat::resolvedOptions() noexcept {
  return optionsFromJava(native(this)->resolvedOptions());
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  // I don't believe the Java logic can throw an exception (the JS
  // method can, but the errors all come from the Intl.cpp logic).  If
  // I am incorrect, this will need to add a try/catch and take a
  // runtime to call raiseRangeError on it.  This is true for all the
  // format methods.
  return stringFromJava(native(this)->format(jsTimeValue));
}

std::vector<Part> DateTimeFormat::formatToParts(double jsTimeValue) noexcept {
  return partsFromJava(native(this)->formatToParts(jsTimeValue));
}

namespace {

class JNumberFormat : public jni::JavaClass<JNumberFormat> {
 public:
  static constexpr auto kJavaDescriptor =
      "Lcom/facebook/hermes/intl/NumberFormat;";

  static jni::local_ref<javaobject> create(
      jni::alias_ref<JLocalesList> locales,
      jni::alias_ref<JOptionsMap> options) {
    return newInstance(locales, options);
  }

  static jni::local_ref<JLocalesList> supportedLocalesOf(
      jni::alias_ref<JLocalesList> locales,
      jni::alias_ref<JOptionsMap> options) {
    static const auto method =
        javaClassStatic()
            ->getStaticMethod<jni::local_ref<JLocalesList>(
                jni::alias_ref<JLocalesList> locales,
                jni::alias_ref<JOptionsMap> options)>("supportedLocalesOf");
    return method(javaClassStatic(), locales, options);
  }

  jni::local_ref<JOptionsMap> resolvedOptions() {
    static const auto method =
        javaClassStatic()->getMethod<jni::local_ref<JOptionsMap>()>(
            "resolvedOptions");
    return method(self());
  }

  jni::local_ref<jstring> format(double jsTimeValue) {
    static const auto method =
        javaClassStatic()->getMethod<jni::alias_ref<jstring>(double)>("format");
    return method(self(), jsTimeValue);
  }

  jni::local_ref<JPartsList> formatToParts(double jsTimeValue) {
    static const auto method =
        javaClassStatic()->getMethod<jni::alias_ref<JPartsList>(double)>(
            "formatToParts");
    return method(self(), jsTimeValue);
  }
};

} // namespace

NumberFormat::NumberFormat() = default;

NumberFormat::~NumberFormat() = default;

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return localesFromJava(
        runtime,
        JNumberFormat::supportedLocalesOf(
            localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

template <>
struct IntlBridgeTraits<NumberFormat> {
  using NativeType = JNumberFormat;
};

vm::CallResult<std::unique_ptr<NumberFormat>> NumberFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return std::make_unique<IntlBridge<NumberFormat>>(
        localesToJava(locales), optionsToJava(options));
  } catch (const std::exception &ex) {
    return runtime.raiseRangeError(ex.what());
  }
}

Options NumberFormat::resolvedOptions() noexcept {
  return optionsFromJava(native(this)->resolvedOptions());
}

std::u16string NumberFormat::format(double number) noexcept {
  // I don't believe the Java logic can throw an exception (the JS
  // method can, but the errors all come from the Intl.cpp logic).  If
  // I am incorrect, this will need to add a try/catch and take a
  // runtime to call raiseRangeError on it.  This is true for all the
  // format methods.
  return stringFromJava(native(this)->format(number));
}

std::vector<Part> NumberFormat::formatToParts(double number) noexcept {
  return partsFromJava(native(this)->formatToParts(number));
}

} // namespace platform_intl
} // namespace hermes
