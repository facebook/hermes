// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermesc -fstatic-require -commonjs -emit-binary %s 2>&1 > /dev/null | %FileCheck --match-full-lines %s
// TODO(T53144040) Fix LIT tests on Windows
// XFAIL: windows

foo(require);
//CHECK: {{.*}}cjs-static-fail-1.js:10:4: warning: 'require' used as function call argument
//CHECK-NEXT: foo(require);
//CHECK-NEXT:    ^

require(require);
//CHECK: {{.*}}cjs-static-fail-1.js:15:8: warning: 'require' used as function call argument
//CHECK-NEXT: require(require);
//CHECK-NEXT:        ^

new require("a");
//CHECK: {{.*}}cjs-static-fail-1.js:20:12: warning: 'require' used as a constructor
//CHECK-NEXT: new require("a");
//CHECK-NEXT:            ^

this.prop1 = require;
//CHECK: {{.*}}cjs-static-fail-1.js:25:12: warning: 'require' escapes or is modified
//CHECK-NEXT: this.prop1 = require;
//CHECK-NEXT:            ^

require();
//CHECK: {{.*}}cjs-static-fail-1.js:30:8: warning: require() invoked without arguments
//CHECK-NEXT: require();
//CHECK-NEXT:        ^

require("b", "c");
//CHECK: {{.*}}cjs-static-fail-1.js:35:8: warning: Additional require() arguments will be ignored
//CHECK-NEXT: require("b", "c");
//CHECK-NEXT:        ^

require(foo + 1);
//CHECK: {{.*}}cjs-static-fail-1.js:40:8: warning: require() argument cannot be coerced to constant string at compile time
//CHECK-NEXT: require(foo + 1);
//CHECK-NEXT:        ^
//CHECK: {{.*}}cjs-static-fail-1.js:40:9: note: First argument of require()
//CHECK-NEXT: require(foo + 1);
//CHECK-NEXT:         ^

function func() {
    require("c");
    var r = require;
//CHECK: {{.*}}cjs-static-fail-1.js:50:11: warning: 'require' is copied to a variable which cannot be analyzed
//CHECK-NEXT:     var r = require;
//CHECK-NEXT:           ^
}

print({}[require]);
//CHECK: {{.*}}cjs-static-fail-1.js:56:9: warning: 'require' is used as a property key and cannot be analyzed
//CHECK-NEXT: print({}[require]);
//CHECK-NEXT:         ^

function loadFromRequire() {
  // No warning should be issued for loading a property.
  print(require.context);
// CHECK-NOT: warning
}

exports.func = func;
