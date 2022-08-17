/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -fno-inline %s | %FileCheck --match-full-lines %s

function foo() {
  throw new Error("qwerty");
}

try { foo() } catch (e) { print(e.stack) }
// CHECK: Error: qwerty
// CHECK-NEXT:     at foo ({{.*}}/stacktrace-bound.js:11:18)
// CHECK-NEXT:     at global ({{.*}}/stacktrace-bound.js:14:10)

try { foo.bind(null)() } catch (e) { print(e.stack) }
// CHECK: Error: qwerty
// CHECK-NEXT:     at foo ({{.*}}/stacktrace-bound.js:11:18)
// CHECK-NEXT:     at global ({{.*}}/stacktrace-bound.js:19:21)

try { foo.bind(null).bind(null)() } catch (e) { print(e.stack) }
// CHECK: Error: qwerty
// CHECK-NEXT:     at foo ({{.*}}/stacktrace-bound.js:11:18)
// CHECK-NEXT:     at global ({{.*}}/stacktrace-bound.js:24:32)

function chain1() {
  chain2bound();
}

function chain2() {
  throw new Error("asdf");
}

var chain2bound = chain2.bind(null);

try { chain1.bind(null)() } catch (e) { print(e.stack) }
// CHECK: Error: asdf
// CHECK-NEXT:     at chain2 ({{.*}}/stacktrace-bound.js:34:18)
// CHECK-NEXT:     at chain1 ({{.*}}/stacktrace-bound.js:30:14)
// CHECK-NEXT:     at global ({{.*}}/stacktrace-bound.js:39:24)

function bar() {
  throw new Error("oops");
}
bar.displayName = "MyComponent";

try { bar.bind(null)(); } catch (e) { print(e.stack); }
//CHECK-LABEL: Error: oops
//CHECK-NEXT:     at MyComponent ({{.*}}stacktrace-bound.js:46:18)
