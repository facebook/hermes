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
+ (NSString *)bestAvailableLocale:(NSString *)locale:(NSArray<NSString *> *)availableLocales {
    // 1. Let candidate be locale
    NSString *candidate = locale;

    // 2. Repeat
    while(true) {
        // a. If availableLocales contains an element equal to candidate, return candidate.
        if ([availableLocales containsObject:candidate]) {
            return candidate;
        }
               
        // b. Let pos be the character index of the last occurrence of "-" (U+002D) within candidate.
        NSUInteger pos = [candidate rangeOfString:@"-" options:NSBackwardsSearch].location;
        // ...If that character does not occur, return undefined.
        if (pos < 0ul) {
            return @"";
        }

        // c. If pos ≥ 2 and the character "-" occurs at index pos-2 of candidate, decrease pos by 2.
        if (pos >= 2ul && [@"-" isEqualToString:[candidate substringWithRange:NSMakeRange(pos-2, 1)]]){
            pos-=2;
        }
        
        // d. Let candidate be the substring of candidate from position 0, inclusive, to position pos, exclusive.
        candidate = [candidate substringToIndex:pos];
    }
}
