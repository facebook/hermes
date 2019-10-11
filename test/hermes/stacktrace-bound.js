// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -fno-inline %s | %FileCheck --match-full-lines %s

function foo() {
  throw new Error("qwerty");
}

try { foo() } catch (e) { print(e.stack) }
// CHECK: Error: qwerty
// CHECK-NEXT:     at foo ({{.*}}/stacktrace-bound.js:9:18)
// CHECK-NEXT:     at global ({{.*}}/stacktrace-bound.js:12:10)

try { foo.bind(null)() } catch (e) { print(e.stack) }
// CHECK: Error: qwerty
// CHECK-NEXT:     at foo ({{.*}}/stacktrace-bound.js:9:18)
// CHECK-NEXT:     at global ({{.*}}/stacktrace-bound.js:17:21)

try { foo.bind(null).bind(null)() } catch (e) { print(e.stack) }
// CHECK: Error: qwerty
// CHECK-NEXT:     at foo ({{.*}}/stacktrace-bound.js:9:18)
// CHECK-NEXT:     at global ({{.*}}/stacktrace-bound.js:22:32)

function chain1() {
  chain2bound();
}

function chain2() {
  throw new Error("asdf");
}

var chain2bound = chain2.bind(null);

try { chain1.bind(null)() } catch (e) { print(e.stack) }
// CHECK: Error: asdf
// CHECK-NEXT:     at chain2 ({{.*}}/stacktrace-bound.js:32:18)
// CHECK-NEXT:     at chain1 ({{.*}}/stacktrace-bound.js:28:14)
// CHECK-NEXT:     at global ({{.*}}/stacktrace-bound.js:37:24)
