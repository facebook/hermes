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

// Amete Alem must affect formatting, not just resolvedOptions().
var ethioaa = new Intl.DateTimeFormat('en-US-u-ca-ethioaa', {
  timeZone: 'UTC',
  year: 'numeric',
});
print(ethioaa.resolvedOptions().calendar, ethioaa.format(Date.UTC(2020, 0, 2)));
// CHECK-NEXT: ethioaa 7512 AA

// The calendar option must reach the formatter without changing the resolved
// locale or losing its other extensions. It must also affect the format
// template so that the era is included.
var ethioaaOption = new Intl.DateTimeFormat('en-US', {
  calendar: 'ethioaa',
  timeZone: 'UTC',
  year: 'numeric',
});
print(
  ethioaaOption.resolvedOptions().locale,
  ethioaaOption.resolvedOptions().calendar,
  ethioaaOption.format(Date.UTC(2020, 0, 2)),
);
// CHECK-NEXT: en-US ethioaa 7512 AA

var gregoryOverride = new Intl.DateTimeFormat('en-US-u-ca-buddhist-hc-h23', {
  calendar: 'gregory',
  timeZone: 'UTC',
  year: 'numeric',
});
print(
  gregoryOverride.resolvedOptions().locale,
  gregoryOverride.resolvedOptions().calendar,
  gregoryOverride.format(Date.UTC(2020, 0, 2)),
);
// CHECK-NEXT: en-US-u-hc-h23 gregory 2020

// Also exercise the dateStyle path, which does not use a custom format template.
var ethioaaStyle = new Intl.DateTimeFormat('en-US-u-ca-gregory', {
  calendar: 'ethioaa',
  timeZone: 'UTC',
  dateStyle: 'short',
});
print(
  ethioaaStyle.resolvedOptions().locale,
  ethioaaStyle.resolvedOptions().calendar,
  ethioaaStyle.format(Date.UTC(2020, 0, 2)),
);
// CHECK-NEXT: en-US ethioaa 4/23/7512 AA
