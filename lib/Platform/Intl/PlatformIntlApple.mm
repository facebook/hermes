/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "hermes/Platform/Intl/PlatformIntl.h"

#import <Foundation/Foundation.h>

NSString *u16StringToNSString(std::u16string src) {
  auto size = src.size();
  if (size == 0) {
    return nil;
  }
  auto cString = (const unichar*)src.c_str();
  if (cString == nullptr) {
    return nil;
  }
  return [NSString stringWithCharacters:cString length:size];
}

std::u16string nsStringToU16String(NSString *src) {
  auto result = std::u16string();
  auto constexpr divisor = sizeof(char16_t) / sizeof(char);
  if ([src canBeConvertedToEncoding:NSUTF16StringEncoding]) {
    auto size = [src lengthOfBytesUsingEncoding:NSUTF16StringEncoding] + 1; // for NULL terminator;
    auto buffer = (char *)malloc(size);
    auto success = [src getCString:buffer maxLength:size encoding:NSUTF16StringEncoding];
    if (success) {
      result = std::u16string((char16_t *)buffer, size / divisor);
    }
    free(buffer);
    return result;
  } else {
    auto data = [src dataUsingEncoding:NSUTF16StringEncoding allowLossyConversion:YES];
    result = std::u16string((char16_t *)data.bytes, data.length / divisor);
  }
  return result;
}

namespace hermes {
namespace platform_intl {

// Implementation of https://tc39.es/ecma402/#sec-canonicalizelocalelist
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
    // 1. If locales is undefined, then
    // Return a new empty List.
    if (locales.size() == 0) {
        return std::vector<std::u16string>();
    }
    // Note:: Some other major input validation occurs closer to VM in 'normalizeLocales' in
    // JSLib/Intl.cpp
    // 2. Let seen be a new empty List.
    std::vector<std::u16string> seen;
    // 3. If Type(locales) is String or Type(locales) is Object and locales has an
    // [[InitializedLocale]] internal slot, then
    // 4. Else
    // We don't yet support Locale object - https://tc39.es/ecma402/#locale-objects
    // As of now, 'locales' can only be a string list/array.
    // 'O' is not a string array of locales
    // 5. Let len be ? ToLength(? Get(O, "length")).
    // 6. Let k be 0.
    // 7. Repeat, while k < len
    for(std::u16string locale : locales) {
        // We don't have steps for 7a. 7b. 7c. i-iv  .. as we only allow string arrays here..
        // Smoke validation.
        // Throw RangeError if input locale string is (1) empty (2) non-ASCII string.
        if (locale.empty()) {
            return runtime->raiseRangeError("Incorrect locale information provided");
        }
        if (std::find(locale.begin(), locale.end(), '_') != locale.end()){
            return runtime->raiseRangeError("Incorrect locale information provided");
        }
        // 7.c.v & 7.c.vi
        auto tempLocale = u16StringToNSString(locale);
        NSString *tempCanonicalizedTag = [NSLocale canonicalLocaleIdentifierFromString:tempLocale];
        auto canonicalizedTag = nsStringToU16String(tempCanonicalizedTag);
        // 7.c.vii
        if(!canonicalizedTag.empty() && std::find(seen.begin(), seen.end(), canonicalizedTag) == seen.end()) {
            seen.push_back(canonicalizedTag);
        }
    }
        return seen;
    }
    // Implementer note: This method corresponds roughly to
    // https://tc39.es/ecma402/#sec-canonicalizelocalelist
    //
    // Also see the implementer notes on DateTimeFormat#DateTimeFormat()
    // for more discussion of locales and CanonicalizeLocaleList.
    //1. Let O be RequireObjectCoercible(this value).
    //2. Let S be ? ToString(O).
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
    DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
    DateTimeFormat::~DateTimeFormat() {}
    vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
        vm::Runtime *runtime,
        const std::vector<std::u16string> &locales,
        const Options &options) noexcept {
      return std::vector<std::u16string>{u"en-CA", u"de-DE"};
    }
    vm::ExecutionStatus DateTimeFormat::initialize(
        vm::Runtime *runtime,
        const std::vector<std::u16string> &locales,
        const Options &options) noexcept {
      impl_->locale = u"en-US";
      return vm::ExecutionStatus::RETURNED;
    }
    Options DateTimeFormat::resolvedOptions() noexcept {
      Options options;
      options.emplace(u"locale", Option(impl_->locale));
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
