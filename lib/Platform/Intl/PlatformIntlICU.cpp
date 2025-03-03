/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/PlatformIntl.h"
#include "impl_icu/Collator.h"
#include "impl_icu/DateTimeFormat.h"
#include "impl_icu/LocaleBCP47Object.h"

// ICU changed the default UChar type on version 59, but we still need to
// support 52+ However, ICU allows us to manually set a type for UChar using
// UCHAR_TYPE so UChar isn't platform dependent.
#define UCHAR_TYPE char16_t

namespace hermes {
namespace platform_intl {
namespace {

// T is a type of the intl service implementation classes.
// BaseT is the base platform_intl class that the intl service
// implementation class inherits from.
// Instance creation and initialization across the intl service
// implementation classes is the same. This function template
// provides the common code.
template <typename T, typename BaseT>
vm::CallResult<std::unique_ptr<BaseT>> createInstance(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  auto instance = std::make_unique<T>();
  if (LLVM_UNLIKELY(
          instance->initialize(runtime, locales, options) ==
          vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  return std::move(instance);
}

} // namespace

// https://tc39.es/ecma402/#sec-intl.getcanonicallocales
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales) {
  // 1. Let ll be ? CanonicalizeLocaleList(locales).
  auto localeBcp47ObjectsRes =
      impl_icu::LocaleBCP47Object::canonicalizeLocaleList(runtime, locales);
  if (LLVM_UNLIKELY(localeBcp47ObjectsRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }
  // 2. Return CreateArrayFromList(ll).
  std::vector<std::u16string> canonicalLocales;
  for (const auto &localeBcp47Object : *localeBcp47ObjectsRes) {
    canonicalLocales.push_back(localeBcp47Object.getCanonicalizedLocaleId());
  }
  return canonicalLocales;
}

// Not yet implemented.
vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"lowered");
}
// Not yet implemented.
vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"uppered");
}

Collator::Collator() = default;
Collator::~Collator() = default;

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return impl_icu::Collator::supportedLocalesOf(runtime, locales, options);
}

vm::CallResult<std::unique_ptr<Collator>> Collator::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return createInstance<impl_icu::Collator, Collator>(
      runtime, locales, options);
}

Options Collator::resolvedOptions() noexcept {
  return static_cast<impl_icu::Collator *>(this)->resolvedOptions();
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return static_cast<impl_icu::Collator *>(this)->compare(x, y);
}

DateTimeFormat::DateTimeFormat() = default;
DateTimeFormat::~DateTimeFormat() = default;

/// https://402.ecma-international.org/8.0/#sec-intl.datetimeformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return impl_icu::DateTimeFormat::supportedLocalesOf(
      runtime, locales, options);
}

vm::CallResult<std::unique_ptr<DateTimeFormat>> DateTimeFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return createInstance<impl_icu::DateTimeFormat, DateTimeFormat>(
      runtime, locales, options);
}

Options DateTimeFormat::resolvedOptions() noexcept {
  return static_cast<impl_icu::DateTimeFormat *>(this)->resolvedOptions();
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  return static_cast<impl_icu::DateTimeFormat *>(this)->format(jsTimeValue);
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
DateTimeFormat::formatToParts(double jsTimeValue) noexcept {
  return static_cast<impl_icu::DateTimeFormat *>(this)->formatToParts(
      jsTimeValue);
}

vm::CallResult<std::u16string> DateTimeFormat::formatRange(
    vm::Runtime &runtime,
    double startDate,
    double endDate) noexcept {
  return static_cast<impl_icu::DateTimeFormat *>(this)->formatRange(
      runtime, startDate, endDate);
}

vm::CallResult<std::vector<Part>> DateTimeFormat::formatRangeToParts(
    vm::Runtime &runtime,
    double startDate,
    double endDate) noexcept {
  return static_cast<impl_icu::DateTimeFormat *>(this)->formatRangeToParts(
      runtime, startDate, endDate);
}

namespace {
struct NumberFormatDummy : NumberFormat {
  NumberFormatDummy(const char16_t *l) : locale(l) {}
  std::u16string locale;
};
} // namespace

NumberFormat::NumberFormat() = default;
NumberFormat::~NumberFormat() = default;

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::CallResult<std::unique_ptr<NumberFormat>> NumberFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::make_unique<NumberFormatDummy>(u"en-US");
}

Options NumberFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(
      u"locale", Option(static_cast<NumberFormatDummy *>(this)->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

std::u16string NumberFormat::format(double number) noexcept {
  auto s = std::to_string(number);
  return std::u16string(s.begin(), s.end());
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
NumberFormat::formatToParts(double number) noexcept {
  std::unordered_map<std::u16string, std::u16string> part;
  part[u"type"] = u"integer";
  // This isn't right, but I didn't want to do more work for a stub.
  std::string s = std::to_string(number);
  part[u"value"] = {s.begin(), s.end()};
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{part};
}

} // namespace platform_intl
} // namespace hermes
