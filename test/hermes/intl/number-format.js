/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// REQUIRES: intl

print(new Intl.NumberFormat('en-US', {style: 'currency', currency: 'EUR', currencyDisplay: 'name'}).format(2));
// CHECK: 2.00 euros

try { Intl.NumberFormat.prototype.resolvedOptions.call(new Intl.DateTimeFormat()) }
catch (e) { print(e) }
// CHECK-NEXT: TypeError: Intl.NumberFormat.prototype.resolvedOptions called with incompatible 'this'
