/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
#if HERMES_PLATFORM_INTL == HERMES_PLATFORM_INTL_ANDROID

#include <fbjni/fbjni.h>

using namespace ::facebook;
using namespace ::hermes;

namespace hermes {
namespace platform_intl {

namespace {

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

jni::local_ref<jstring> stringToJava(std::u16string str) {
  return jni::make_jstring(str);
}

jni::local_ref<JLocalesList> localesToJava(
    std::vector<std::u16string> locales) {
  jni::local_ref<JArrayList<jni::JString>> ret =
      JArrayList<jni::JString>::create(locales.size());
  for (const auto &locale : locales) {
    ret->add(jni::make_jstring(locale));
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
      jvalue = jni::make_jstring(kv.second.getString());
    }
    ret->put(jni::make_jstring(kv.first), jvalue);
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
    vm::Runtime *runtime,
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
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  try {
    return localesFromJava(
        runtime, JIntl::getCanonicalLocales(localesToJava(locales)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
  }
}

vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  try {
    return stringFromJava(
        JIntl::toLocaleLowerCase(localesToJava(locales), stringToJava(str)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
  }
}

vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  try {
    return stringFromJava(
        JIntl::toLocaleUpperCase(localesToJava(locales), stringToJava(str)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
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

struct Collator::Impl {
  jni::global_ref<JCollator> jCollator_;
};

Collator::Collator() : impl_(std::make_unique<Impl>()) {}

Collator::~Collator() {}

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return localesFromJava(
        runtime,
        JCollator::supportedLocalesOf(
            localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
  }
}

vm::ExecutionStatus Collator::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    impl_->jCollator_ = jni::make_global(
        JCollator::create(localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
  }

  return vm::ExecutionStatus::RETURNED;
}

Options Collator::resolvedOptions() noexcept {
  return optionsFromJava(impl_->jCollator_->resolvedOptions());
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return impl_->jCollator_->compare(stringToJava(x), stringToJava(y));
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

struct DateTimeFormat::Impl {
  jni::global_ref<JDateTimeFormat> jDateTimeFormat_;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}

DateTimeFormat::~DateTimeFormat() {}

vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return localesFromJava(
        runtime,
        JDateTimeFormat::supportedLocalesOf(
            localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
  }
}

vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    impl_->jDateTimeFormat_ = jni::make_global(JDateTimeFormat::create(
        localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
  }

  return vm::ExecutionStatus::RETURNED;
}

Options DateTimeFormat::resolvedOptions() noexcept {
  return optionsFromJava(impl_->jDateTimeFormat_->resolvedOptions());
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  // I don't believe the Java logic can throw an exception (the JS
  // method can, but the errors all come from the Intl.cpp logic).  If
  // I am incorrect, this will need to add a try/catch and take a
  // runtime to call raiseRangeError on it.  This is true for all the
  // format methods.
  return stringFromJava(impl_->jDateTimeFormat_->format(jsTimeValue));
}

std::vector<Part> DateTimeFormat::formatToParts(double jsTimeValue) noexcept {
  return partsFromJava(impl_->jDateTimeFormat_->formatToParts(jsTimeValue));
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

struct NumberFormat::Impl {
  jni::global_ref<JNumberFormat> jNumberFormat_;
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}

NumberFormat::~NumberFormat() {}

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    return localesFromJava(
        runtime,
        JNumberFormat::supportedLocalesOf(
            localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
  }
}

vm::ExecutionStatus NumberFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  try {
    impl_->jNumberFormat_ = jni::make_global(
        JNumberFormat::create(localesToJava(locales), optionsToJava(options)));
  } catch (const std::exception &ex) {
    return runtime->raiseRangeError(ex.what());
  }

  return vm::ExecutionStatus::RETURNED;
}

Options NumberFormat::resolvedOptions() noexcept {
  return optionsFromJava(impl_->jNumberFormat_->resolvedOptions());
}

std::u16string NumberFormat::format(double number) noexcept {
  // I don't believe the Java logic can throw an exception (the JS
  // method can, but the errors all come from the Intl.cpp logic).  If
  // I am incorrect, this will need to add a try/catch and take a
  // runtime to call raiseRangeError on it.  This is true for all the
  // format methods.
  return stringFromJava(impl_->jNumberFormat_->format(number));
}

std::vector<Part> NumberFormat::formatToParts(double number) noexcept {
  return partsFromJava(impl_->jNumberFormat_->formatToParts(number));
}

} // namespace platform_intl
} // namespace hermes

#endif
