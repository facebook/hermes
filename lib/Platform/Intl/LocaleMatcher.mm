/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#import "LocaleMatcher.h"

@implementation LocaleMatcher

#pragma mark - Class Methods

// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-bestavailablelocale
+ (NSString *)bestAvailableLocale:(NSString *)
                           locale:(NSArray<NSString *> *)availableLocales {
  // 1. Let candidate be locale
  NSString *candidate = locale;

  // 2. Repeat
  while (true) {
    // a. If availableLocales contains an element equal to candidate, return
    // candidate.
    if ([availableLocales containsObject:candidate]) {
      return candidate;
    }

    // b. Let pos be the character index of the last occurrence of "-" (U+002D)
    // within candidate.
    NSUInteger pos =
        [candidate rangeOfString:@"-" options:NSBackwardsSearch].location;
    // ...If that character does not occur, return undefined.
    if (pos < 0ul) {
      return @"";
    }

    // c. If pos ≥ 2 and the character "-" occurs at index pos-2 of candidate,
    // decrease pos by 2.
    if (pos >= 2ul &&
        [@"-" isEqualToString:[candidate substringWithRange:NSMakeRange(
                                                                pos - 2, 1)]]) {
      pos -= 2;
    }

    // d. Let candidate be the substring of candidate from position 0,
    // inclusive, to position pos, exclusive.
    candidate = [candidate substringToIndex:pos];
  }
}
// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-lookupmatcher
+ (NSString *)LocaleMatchResult:(NSString *)
                         locale:(NSArray<NSString *> *)requestedLocales,
                                NSArray<NSString *> *availableLocales {
  //     1. Let result be a new Record.
  NSString *result;
  //     2. For each element locale of requestedLocales, do
  for (NSString *locale : requestedLocales) {
    // TODO a. Let noExtensionsLocale be the String value that is locale with
    // any Unicode locale extension sequences removed. b. Let availableLocale be
    // BestAvailableLocale(availableLocales, noExtensionsLocale).
    NSString *availableLocale = bestAvailableLocale(availableLocales, locale);
    // c. If availableLocale is not undefined, then
    if ([availableLocale length] != 0) {
      // i. Set result.[[locale]] to availableLocale.
      result = availableLocale;
      // TODO
      // ii. If locale and noExtensionsLocale are not the same String value,
      // then
      // 1. Let extension be the String value consisting of the substring of the
      // Unicode locale extension sequence within locale.
      // 2. Set result.[[extension]] to extension.

      // iii. Return result.
      return result;
    }
    // 3. Let defLocale be DefaultLocale().
    // 4. Set result.[[locale]] to defLocale.
    // 5. Return result.
    return nsStringToU16String([NSLocale currentLocale]);
  }
