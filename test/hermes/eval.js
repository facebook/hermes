/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=en_US.UTF-8 %hermes -O -Wno-direct-eval %s | %FileCheck --match-full-lines %s
// RUN: LC_ALL=en_US.UTF-8 %hermes -O -optimized-eval -Wno-direct-eval %s | %FileCheck --match-full-lines %s
"use strict";

print('eval');
// CHECK-LABEL: eval

print(eval("print(123)"));
// CHECK: 123
// CHECK: undefined

print(eval("eval(1 + 2)"));
// CHECK: 3

var x = 2;
print(eval("x + 3"));
// CHECK: 5
print(eval(x + "+ 3"));
// CHECK: 5

print(eval("x = x + 3"));
// CHECK: 5
print(eval('x'));
// CHECK: 5
print(eval('x'));
// CHECK: 5

print(eval('"à á â"'));
// CHECK: à á â

var f = eval('(function() {return 1})')
print(f());
// CHECK: 1

print('end');
// CHECK: end

try { eval('function()'); } catch(e) { print('caught', e.name, e.message) }
// CHECK-NEXT: caught SyntaxError {{.*}}

eval("print('asdf\0hjkl'.length);");
// CHECK-NEXT: 9
