/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "hermes/Platform/Intl/PlatformIntlShared.h"

#include <deque>
#include <string>
#include <unordered_map>
#include "unicode/udat.h"
#include "llvh/Support/ConvertUTF.h"

using namespace ::facebook;
using namespace ::hermes;

namespace hermes {
namespace platform_intl {
namespace {
  vm::CallResult<std::u16string> UTF8toUTF16(vm::Runtime &runtime, std::string_view in) {
    std::u16string out;
    size_t length = in.length();
    out.resize(length);
    const llvh::UTF8 *sourceStart = reinterpret_cast<const llvh::UTF8 *>(&in[0]);
    const llvh::UTF8 *sourceEnd = sourceStart + length;
    llvh::UTF16 *targetStart = reinterpret_cast<llvh::UTF16 *>(&out[0]);
    llvh::UTF16 *targetEnd = targetStart + out.size();
    llvh::ConversionResult convRes = ConvertUTF8toUTF16(
        &sourceStart,
        sourceEnd,
        &targetStart,
        targetEnd,
        llvh::lenientConversion);
    if (convRes != llvh::ConversionResult::conversionOK) {
      return runtime.raiseRangeError("utf8 to utf16 conversion failed");
    }
    out.resize(reinterpret_cast<char16_t *>(targetStart) - &out[0]);
    return out;
  }

  const std::vector<std::u16string> &getAvailableLocales(vm::Runtime &runtime) {
    static const std::vector<std::u16string> *availableLocales = [&runtime] {
      auto *vec = new std::vector<std::u16string>();

      for (int32_t i = 0, count = uloc_countAvailable(); i < count; i++) {
        auto locale = uloc_getAvailable(i);
        vec->push_back(UTF8toUTF16(runtime, locale).getValue());
      }

      return vec;
    }();

    return *availableLocales;
  }

} // namespace

// https://tc39.es/ecma402/#sec-intl.getcanonicallocales
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales) {
  return canonicalizeLocaleList(runtime, locales);
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

namespace {
struct CollatorDummy : Collator {
  CollatorDummy(const char16_t *l) : locale(l) {}
  std::u16string locale;
};
} // namespace

Collator::Collator() = default;
Collator::~Collator() = default;

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::CallResult<std::unique_ptr<Collator>> Collator::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::make_unique<CollatorDummy>(u"en-US");
}

Options Collator::resolvedOptions() noexcept {
  Options options;
  options.emplace(
      u"locale", Option(static_cast<CollatorDummy *>(this)->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return x.compare(y);
}

namespace {
// Implementation of
// https://402.ecma-international.org/8.0/#datetimeformat-objects
struct DateTimeFormatICU : DateTimeFormat {
  DateTimeFormatICU(const char16_t *l) : locale(l) {}
  std::u16string locale;
};
} // namespace

DateTimeFormat::DateTimeFormat() = default;
DateTimeFormat::~DateTimeFormat() = default;

// Implementation of
// https://402.ecma-international.org/8.0/#sec-intl.datetimeformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let availableLocales be %DateTimeFormat%.[[AvailableLocales]].
  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  auto requestedLocales = getCanonicalLocales(runtime, locales);
  const std::vector<std::u16string> &availableLocales = getAvailableLocales(runtime);
  // 3. Return ? (availableLocales, requestedLocales, options).
  return supportedLocales(availableLocales, requestedLocales.getValue());
}

vm::CallResult<std::unique_ptr<DateTimeFormat>> DateTimeFormat::create(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::make_unique<DateTimeFormatICU>(u"en-US");
}

Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(
      u"locale", Option(static_cast<DateTimeFormatICU *>(this)->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  auto s = std::to_string(jsTimeValue);
  return std::u16string(s.begin(), s.end());
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
DateTimeFormat::formatToParts(double jsTimeValue) noexcept {
  std::unordered_map<std::u16string, std::u16string> part;
  part[u"type"] = u"integer";
  // This isn't right, but I didn't want to do more work for a stub.
  std::string s = std::to_string(jsTimeValue);
  part[u"value"] = {s.begin(), s.end()};
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{part};
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
