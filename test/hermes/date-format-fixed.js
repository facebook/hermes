/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
RUN: TZ=EST+5 LC_ALL=en_US _HERMES_TEST_LOCALE=en_US %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines %s
RUN: TZ=EST+5 LC_ALL=tr_TR _HERMES_TEST_LOCALE=tr_TR %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines %s
RUN: TZ=EST+5 LC_ALL=C %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines %s
RUN: TZ=EST+5 LC_ALL=tr_TR _HERMES_TEST_LOCALE=tr_TR %shermes -exec %s \
RUN:            | %FileCheck --match-full-lines %s
UNSUPPORTED: locale_aware_dates || intl
*/

// The backends without a platform date formatter emit a fixed English format
// that does not depend on the host locale. All runs above use the same CHECK
// lines deliberately: that is the contract being tested. This does catch a
// formatter that grew locale sensitivity the way the rest of this file does
// -- reading _HERMES_TEST_LOCALE, LC_ALL, LC_MESSAGES or LANG via getenv (see
// computeHostCaseLocale in PlatformUnicodeHermes.cpp), or calling setlocale
// -- the tr_TR run would fail. It does not catch a formatter that calls
// strftime for a locale-dependent field without ever calling setlocale:
// glibc's strftime ignores LC_ALL and friends unless setlocale opted in, so
// such a change would pass all four runs above undetected.

"use strict";

print('toLocaleString');
// CHECK-LABEL: toLocaleString
print(new Date(112).toLocaleString());
// CHECK-NEXT: Dec 31, 1969, 7:00:00 PM

print('toLocaleDateString');
// CHECK-LABEL: toLocaleDateString
print(new Date(112).toLocaleDateString());
// CHECK-NEXT: Dec 31, 1969

print('toLocaleTimeString');
// CHECK-LABEL: toLocaleTimeString
print(new Date(112).toLocaleTimeString());
// CHECK-NEXT: 7:00:00 PM

print('Invalid');
// CHECK-LABEL: Invalid
print(new Date(NaN).toLocaleTimeString());
// CHECK-NEXT: Invalid Date
