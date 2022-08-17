/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
RUN: LC_ALL=en_US.UTF-8 _HERMES_TEST_LOCALE=en-US %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines -check-prefix EN %s
RUN: LC_ALL=tr_TR.UTF-8 _HERMES_TEST_LOCALE=tr-TR %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines -check-prefix TR %s
RUN: LC_ALL=UTF-8 %hermes -O -target=HBC %s \
RUN:            | %FileCheck --match-full-lines -check-prefix NO %s
TODO(T53144040) Fix LIT tests on Windows
XFAIL: windows
*/
"use strict";

print('toLocaleLowerCase');
// EN-LABEL: toLocaleLowerCase
// TR-LABEL: toLocaleLowerCase
// NO-LABEL: toLocaleLowerCase
print('ASDF'.toLocaleLowerCase());
// EN-NEXT: asdf
// TR-NEXT: asdf
// NO-NEXT: asdf
print('AEIOU'.toLocaleLowerCase());
// EN-NEXT: aeiou
// TR-NEXT: aeıou
// NO-NEXT: aeiou

print('toLocaleUpperCase');
// EN-LABEL: toLocaleUpperCase
// TR-LABEL: toLocaleUpperCase
// NO-LABEL: toLocaleUpperCase
print('asdf'.toLocaleUpperCase());
// EN-NEXT: ASDF
// TR-NEXT: ASDF
// NO-NEXT: ASDF
print('aeiou'.toLocaleUpperCase());
// EN-NEXT: AEIOU
// TR-NEXT: AEİOU
// NO-NEXT: AEIOU

print('I\u0307'.toLocaleLowerCase());
// EN-NEXT: i̇
// TR-NEXT: i
// NO-NEXT: i̇
