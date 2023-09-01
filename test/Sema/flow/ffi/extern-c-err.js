/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheck --match-full-lines %s

type MyNumber = number;

$SHBuiltin.extern_c();
// CHECK: {{.*}}:[[@LINE-1]]:1: error: ft: extern_c requires exactly two arguments

$SHBuiltin.extern_c(10, 20);
// CHECK: {{.*}}:[[@LINE-1]]:21: error: ft: extern_c requires an object literal as the first argument

$SHBuiltin.extern_c({}, 20);
// CHECK: {{.*}}:[[@LINE-1]]:25: error: ft: extern_c requires a function as the second argument

$SHBuiltin.extern_c({}, function () {});
// CHECK: {{.*}}:[[@LINE-1]]:25: error: ft: extern_c requires a named function as the second argument

$SHBuiltin.extern_c({}, function foo() {});
// CHECK: {{.*}}:[[@LINE-1]]:25: error: ft: extern_c requires a typed function as the second argument

$SHBuiltin.extern_c({}, function *foo():void {});
// CHECK: {{.*}}:[[@LINE-1]]:25: error: ft: extern_c does not support async or generator functions

$SHBuiltin.extern_c({}, async function foo():void {});
// CHECK: {{.*}}:[[@LINE-1]]:25: error: ft: extern_c does not support async or generator functions

$SHBuiltin.extern_c({}, function foo(this: number):void {});
// CHECK: {{.*}}:[[@LINE-1]]:25: error: ft: extern_c does not support 'this' parameters

$SHBuiltin.extern_c({}, function foo(x: c_int) {});
// CHECK: {{.*}}:[[@LINE-1]]:25: error: ft: extern_c requires a return type annotation

$SHBuiltin.extern_c({}, function foo(x: c_int): number  {});
// CHECK: {{.*}}:[[@LINE-1]]:47: error: ft: unsupported native type annotation

$SHBuiltin.extern_c({}, function foo(x: bool): c_int  {});
// CHECK: {{.*}}:[[@LINE-1]]:39: error: ft: unsupported native type annotation

$SHBuiltin.extern_c({}, function foo(x: MyNumber): c_int  {});
// CHECK: {{.*}}:[[@LINE-1]]:41: error: ft: 'MyNumber' is not a native type

$SHBuiltin.extern_c({}, function foo(x: c_int, y): c_int  {});
// CHECK: {{.*}}:[[@LINE-1]]:48: error: ft: extern_c requires type annotations for all parameters


$SHBuiltin.extern_c({}, function func(x: c_int): void {});
$SHBuiltin.extern_c({}, function func(x: c_int): c_int {throw 1});
// CHECK: {{.*}}:[[@LINE-1]]:1: error: ft: invalid redeclaration of native extern 'func'
// CHECK: {{.*}}:[[@LINE-3]]:1: note: ft: original declaration here

