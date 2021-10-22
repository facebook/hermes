/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/PlatformIntl.h"

#import <Foundation/Foundation.h>

namespace hermes {
namespace platform_intl {

std::vector<std::u16string> nsStringArrayToU16StringArray(NSArray<NSString *> * array) {
  auto size = [array count];
  std::vector<std::u16string> result;
  result.reserve(size);
  for (size_t i = 0; i < size; i++) {
    result[i] = nsStringToU16String(array[i]);
  }
  return result;
}

NSArray<NSString *> *u16StringArrayTonsStringArray(std::vector<std::u16string> array) {
  auto size = array.size();
  NSArray<NSString *> * result = [NSArray arrayWithCapacity:size];
  for (size_t i = 0; i < size; i++) {
    result[i] = u16StringToNSString(array[i]);
  }
  return result;
}

vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  return std::vector<std::u16string>{u"fr-FR", u"es-ES"};
}

vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"lowered");
}
vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"uppered");
}

// Implementation of
// https://tc39.es/ecma402/#sec-supportedlocales
vm::CallResult<std::vector<std::u16string>> SupportedLocales(
    std::vector<std::u16string> availableLocales,
    std::vector<std::u16string> requestedLocales,
    const Options &options) {
  return {};
}

struct Collator::Impl {
  std::u16string locale;
};

Collator::Collator() : impl_(std::make_unique<Impl>()) {}
Collator::~Collator() {}

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus Collator::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options Collator::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return x.compare(y);
}

struct DateTimeFormat::Impl {
  std::u16string locale;
};

// Implementation of
// https://tc39.es/ecma402/#datetimeformat-objects
DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}

// Implementation of
// https://tc39.es/ecma402/#sec-intl.datetimeformat.supportedlocalesof
vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  // 1. Let availableLocales be %DateTimeFormat%.[[AvailableLocales]].
  NSArray<NSString *> *nsAvailableLocales = [NSLocale availableLocaleIdentifiers];
  // 2. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  vm::CallResult<std::vector<std::u16string>> requestedLocales = getCanonicalLocales(locales);
  std::vector<std::u16string> availableLocales = nsStringArrayToU16StringArray(nsAvailableLocales);
  // 3. Return ? SupportedLocales(availableLocales, requestedLocales, options).
  return SupportedLocales(availableLocales, requestedLocales, options);
}

vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  //        1. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  vm::CallResult<std::vector<std::u16string>> requestedLocales =
      getCanonicalLocales(runtime, locales);
  //        3. Let opt be a new Record.
  Impl opt;
  //        4. Let matcher be ? GetOption(options, "localeMatcher", "string", «
  //        "lookup", "best fit" », "best fit").
  //        5. Set opt.[[localeMatcher]] to matcher.
  Impl matcher = normalizeOptions(kDTFOptions); // Not right

  //        6. Let calendar be ? GetOption(options, "calendar", "string",
  //        undefined, undefined).
  //        7. If calendar is not undefined, then
  //        a. If calendar does not match the Unicode Locale Identifier type
  //        nonterminal, throw a RangeError exception.
  //        8. Set opt.[[ca]] to calendar.

  //        9. Let numberingSystem be ? GetOption(options, "numberingSystem",
  //        "string", undefined, undefined).
  //        10. If numberingSystem is not undefined, then
  //        a. If numberingSystem does not match the Unicode Locale Identifier
  //        type nonterminal, throw a RangeError exception.
  //        11. Set opt.[[nu]] to numberingSystem.

  //        12. Let hour12 be ? GetOption(options, "hour12", "boolean",
  //        undefined, undefined).
  //        13. Let hourCycle be ? GetOption(options, "hourCycle", "string", «
  //        "h11", "h12", "h23", "h24" », undefined).
  //        14. If hour12 is not undefined, then
  //        a. Let hourCycle be null.
  //        15. Set opt.[[hc]] to hourCycle.
  return vm::ExecutionStatus::RETURNED;
}

Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  // Implementation of
  // https://tc39.es/ecma402/#sec-todatetimeoptions
  //        2. Let options be ? ToDateTimeOptions(options, "any", "date").
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

struct NumberFormat::Impl {
  std::u16string locale;
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}
NumberFormat::~NumberFormat() {}

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus NumberFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options NumberFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
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
