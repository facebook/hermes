/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


// RUN: TZ=US/Pacific %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print("test default timezone");
// CHECK-LABEL: test default timezone

const timeZone = new Intl.DateTimeFormat().resolvedOptions().timeZone;
const options = {
  year: 'numeric', month: 'numeric', day: 'numeric',
  hour: 'numeric', minute: 'numeric', second: 'numeric',
  hour12: false,
  timeZone: timeZone
};
const date = new Date(Date.UTC(2020, 0, 2, 3, 45, 00, 30));
print(new Intl.DateTimeFormat('en-US', options).format(date));
// CHECK-NEXT: 1/1/2020, 19:45:00
