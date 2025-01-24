/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// REQUIRES: regexp_unicode_properties

// Ensure that parsing a regex which contains an inversion of
// the very end of the valid unicode space does not segfault.

var re = /[\P{Any}|x]/u;

print("regex regress");
// CHECK:regex regress

print(re.exec("a") == null);
// CHECK-NEXT:true

// Ensure that the "Assigned" category was generated correctly.
re = /[\P{Assigned}]/u;
print(re.exec("\u0378") == null);
// CHECK-NEXT:false
