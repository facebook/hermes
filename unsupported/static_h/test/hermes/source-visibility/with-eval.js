/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

print("with eval");
// CHECK-LABEL: with eval

var dflt = eval("(function(){})");
print(dflt.toString());
// CHECK-NEXT: function () { [bytecode] }

var showSource = eval("(function(){'show source'})");
print(showSource.toString());
// CHECK-NEXT: function(){'show source'}

var hideSource = eval("(function(){'hide source'})");
print(hideSource.toString());
// CHECK-NEXT: function () { [native code] }

var sensitive = eval("(function(){'sensitive'})");
print(sensitive.toString());
// CHECK-NEXT: function () { [native code] }

