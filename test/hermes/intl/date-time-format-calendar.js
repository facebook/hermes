/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ=GMT %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: TZ=GMT %shermes -O -exec %s | %FileCheck --match-full-lines %s
// REQUIRES: intl && apple

// ECMA-402 requires [[Calendar]] to always be defined in resolvedOptions().

print('calendar in resolvedOptions');
// CHECK-LABEL: calendar in resolvedOptions

print(new Intl.DateTimeFormat('en-US').resolvedOptions().calendar);
// CHECK-NEXT: gregory

print(new Intl.DateTimeFormat('th-TH').resolvedOptions().calendar);
// CHECK-NEXT: buddhist

print(new Intl.DateTimeFormat('ja-JP-u-ca-japanese').resolvedOptions().calendar);
// CHECK-NEXT: japanese

print(
  new Intl.DateTimeFormat('en-US', {calendar: 'iso8601'}).resolvedOptions()
    .calendar,
);
// CHECK-NEXT: iso8601

// A requested calendar must actually affect formatting.
print(
  new Intl.DateTimeFormat('ja-JP-u-ca-japanese', {dateStyle: 'full'}).format(
    new Date(Date.UTC(2020, 0, 2)),
  ),
);
// CHECK-NEXT: 令和2年1月2日 木曜日

print(
  new Intl.DateTimeFormat('th-TH', {dateStyle: 'full'}).format(
    new Date(Date.UTC(2020, 0, 2)),
  ),
);
// CHECK-NEXT: วันพฤหัสบดีที่ 2 มกราคม พ.ศ. 2563
