/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
RUN: TZ=EST+5 LC_ALL=en_US _HERMES_TEST_LOCALE=en_US %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines -check-prefix US %s
RUN: TZ=EST+5 LC_ALL=tr_TR _HERMES_TEST_LOCALE=tr_TR %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines -check-prefix TR %s
RUN: TZ=EST+5 %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines -check-prefix NO %s
TODO(T53144040) Fix LIT tests on Windows
XFAIL: windows
UNSUPPORTED: ubsan || intl
*/
"use strict";

print('toLocaleString');
// US-LABEL: toLocaleString
// TR-LABEL: toLocaleString
// NO-LABEL: toLocaleString
print(new Date(112).toLocaleString());
// US-NEXT: Dec 31, 1969{{.+}}7:00:00 PM
// TR-NEXT: 31 Ara 1969{{.+}}19:00:00
// NO-NEXT: Dec 31, 1969{{.+}}7:00:00 PM

print('toLocaleDateString');
// US-LABEL: toLocaleDateString
// TR-LABEL: toLocaleDateString
// NO-LABEL: toLocaleDateString
print(new Date(112).toLocaleDateString());
// US-NEXT: Dec 31, 1969
// TR-NEXT: 31 Ara 1969
// NO-NEXT: Dec 31, 1969

print('toLocaleTimeString');
// US-LABEL: toLocaleTimeString
// TR-LABEL: toLocaleTimeString
// NO-LABEL: toLocaleTimeString
print(new Date(112).toLocaleTimeString());
// US-NEXT: 7:00:00 PM
// TR-NEXT: 19:00:00
// NO-NEXT: 7:00:00 PM

print('Invalid');
// US-LABEL: Invalid
// TR-LABEL: Invalid
// NO-LABEL: Invalid
print(new Date(NaN).toLocaleTimeString());
// US-NEXT: Invalid Date
// TR-NEXT: Invalid Date
// NO-NEXT: Invalid Date
