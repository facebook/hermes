/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ=GMT %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: TZ=GMT %shermes -O -exec %s | %FileCheck --match-full-lines %s
// REQUIRES: intl && apple

// ECMA-402 requires style:"unit" to format the number as-is in the requested
// unit, without converting to the locale's preferred unit of the dimension.

print('unit style keeps the requested unit');
// CHECK-LABEL: unit style keeps the requested unit

print(
  new Intl.NumberFormat('en-US', {style: 'unit', unit: 'hour'}).format(2),
);
// CHECK-NEXT: 2 hr

print(
  new Intl.NumberFormat('en-US', {
    style: 'unit',
    unit: 'minute',
    unitDisplay: 'short',
  }).format(2),
);
// CHECK-NEXT: 2 min

print(
  new Intl.NumberFormat('en-US', {
    style: 'unit',
    unit: 'second',
    unitDisplay: 'narrow',
  }).format(2),
);
// CHECK-NEXT: 2s

print(
  new Intl.NumberFormat('en-US', {
    style: 'unit',
    unit: 'hour',
    unitDisplay: 'narrow',
  }).format(2),
);
// CHECK-NEXT: 2h

print(
  new Intl.NumberFormat('en-US', {
    style: 'unit',
    unit: 'hour',
    unitDisplay: 'long',
  }).format(2),
);
// CHECK-NEXT: 2 hours

// en-US prefers imperial units, but the requested metric unit must be kept.
print(
  new Intl.NumberFormat('en-US', {
    style: 'unit',
    unit: 'kilometer',
    unitDisplay: 'short',
  }).format(2),
);
// CHECK-NEXT: 2 km

print(
  new Intl.NumberFormat('en-US', {
    style: 'unit',
    unit: 'liter',
    unitDisplay: 'narrow',
  }).format(2),
);
// CHECK-NEXT: 2L

print(
  new Intl.NumberFormat('en-US', {
    style: 'unit',
    unit: 'kilometer-per-hour',
    unitDisplay: 'short',
  }).format(2),
);
// CHECK-NEXT: 2 km/h

// The number is still formatted by NumberFormat's digit options.
print(
  new Intl.NumberFormat('en-US', {
    style: 'unit',
    unit: 'hour',
    unitDisplay: 'short',
    maximumFractionDigits: 1,
  }).format(1.5),
);
// CHECK-NEXT: 1.5 hr
