/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: TZ=GMT %hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: intl
// UNSUPPORTED: apple

print('additional date time format test');
// CHECK-LABEL: additional date time format test

const date = new Date(Date.UTC(2020, 0, 2, 3, 45, 0, 30));

// Apple implementation does not support offset time zone strings.
print(new Intl.DateTimeFormat('en-US', { timeStyle: 'long', timeZone: '+05:00' }).format(date));
// CHECK-NEXT: 8:45:00{{.+}}AM GMT+5

print(new Intl.DateTimeFormat('en-US', { timeStyle: 'long', timeZone: '-1030' }).format(date));
// CHECK-NEXT: 5:15:00{{.+}}PM GMT-10:30

// Apple implementation does not support numberingSystem option.
print(new Intl.DateTimeFormat('en-US').resolvedOptions().numberingSystem);
// CHECK-NEXT: latn
