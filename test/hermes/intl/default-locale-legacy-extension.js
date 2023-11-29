/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: _HERMES_TEST_LOCALE="en-GB@foo=bar" %hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print(Intl.Collator().resolvedOptions().locale);
// CHECK: en-GB

print(Intl.DateTimeFormat().resolvedOptions().locale);
// CHECK-NEXT: en-GB
