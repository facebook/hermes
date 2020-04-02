/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSLib/RuntimeCommonStorage.h"

#include <memory>

#include "hermes/VM/JSLib.h"

namespace hermes {
namespace vm {

std::shared_ptr<RuntimeCommonStorage> createRuntimeCommonStorage(
    bool shouldTrace) {
  return std::make_shared<RuntimeCommonStorage>(shouldTrace);
}

RuntimeCommonStorage::RuntimeCommonStorage(bool shouldTrace)
    : shouldTrace(shouldTrace) {}
RuntimeCommonStorage::~RuntimeCommonStorage() {}

#ifdef __APPLE__
/// Create the locale used for date formatting and collation. \return the
/// locale, transferring ownership to the caller (the "create" rule).
static CFLocaleRef createLocale() {
  CFLocaleRef localeRef = nullptr;
  // Look for our special environment variable. This is used for testing only.
  const char *hermesLocale = std::getenv("_HERMES_TEST_LOCALE");
  if (hermesLocale) {
    CFStringRef localeName =
        CFStringCreateWithCString(nullptr, hermesLocale, kCFStringEncodingUTF8);
    localeRef = CFLocaleCreate(nullptr, localeName);
    CFRelease(localeName);
  }
  if (!localeRef)
    localeRef = CFLocaleCopyCurrent();
  return localeRef;
}

CFLocaleRef RuntimeCommonStorage::copyLocale() {
  static CFLocaleRef hermesLocale = createLocale();
  return CFLocaleCreateCopy(nullptr, hermesLocale);
}
#endif

} // namespace vm
} // namespace hermes
