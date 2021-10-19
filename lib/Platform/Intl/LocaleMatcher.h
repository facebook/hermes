/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

@interface LocaleMatcher : NSObject

+ (NSString *)bestAvailableLocale:(NSString *)locale:(NSArray<NSString *> *)availableLocales;
+ (NSString *)getBestAvailableLocale:(std::vector<std::u16string>)requestedLocales;

@end
