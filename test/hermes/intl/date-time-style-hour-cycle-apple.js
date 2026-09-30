/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ=GMT %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: TZ=GMT %shermes -O -exec %s | %FileCheck --match-full-lines %s
// REQUIRES: intl && apple

const afternoon = new Date('2026-05-29T14:54:00Z');
const midnight = new Date('2026-05-29T00:54:00Z');

function check(label, locale, options, date) {
  const formatter = new Intl.DateTimeFormat(locale, {
    ...options,
    timeZone: 'UTC',
  });
  const resolved = formatter.resolvedOptions();
  const output = formatter.format(date).replace(/[\u00a0\u202f]/g, ' ');
  print(label, resolved.locale, resolved.hourCycle, resolved.hour12, output);
}

check('12h en-GB', 'en-GB', {timeStyle: 'short', hour12: true}, afternoon);
// CHECK: 12h en-GB en-GB h12 true 2:54 pm

check('12h midnight', 'en-GB', {timeStyle: 'short', hour12: true}, midnight);
// CHECK-NEXT: 12h midnight en-GB h12 true 12:54 am

check('24h en-US', 'en-US', {timeStyle: 'short', hour12: false}, afternoon);
// CHECK-NEXT: 24h en-US en-US h23 false 14:54

check('h11 midnight', 'en-GB', {timeStyle: 'short', hourCycle: 'h11'}, midnight);
// CHECK-NEXT: h11 midnight en-GB h11 true 0:54 am

check('h24 midnight', 'en-GB', {timeStyle: 'short', hourCycle: 'h24'}, midnight);
// CHECK-NEXT: h24 midnight en-GB h24 false 24:54

check('locale extension', 'en-GB-u-hc-h12', {timeStyle: 'short'}, afternoon);
// CHECK-NEXT: locale extension en-GB-u-hc-h12 h12 true 2:54 pm

check('invalid extension', 'en-GB-u-hc-foo', {timeStyle: 'short'}, afternoon);
// CHECK-NEXT: invalid extension en-GB h23 false 14:54

check('empty extension', 'en-GB-u-hc', {timeStyle: 'short'}, afternoon);
// CHECK-NEXT: empty extension en-GB h23 false 14:54

check('hour12 precedence', 'en-GB-u-hc-h24', {
  timeStyle: 'short', hour12: true,
}, midnight);
// CHECK-NEXT: hour12 precedence en-GB h12 true 12:54 am

check('hourCycle precedence', 'en-GB-u-hc-h12', {
  timeStyle: 'short', hourCycle: 'h23',
}, afternoon);
// CHECK-NEXT: hourCycle precedence en-GB h23 false 14:54

check('date and time', 'en-GB', {
  dateStyle: 'medium', timeStyle: 'short', hour12: true,
}, afternoon);
// CHECK-NEXT: date and time en-GB h12 true 29 May 2026 at 2:54 pm

check('date only', 'en-GB', {dateStyle: 'medium', hour12: true}, afternoon);
// CHECK-NEXT: date only en-GB undefined undefined 29 May 2026

check('ja-JP midnight', 'ja-JP', {timeStyle: 'short', hour12: true}, midnight);
// CHECK-NEXT: ja-JP midnight ja-JP h11 true 午前0:54
