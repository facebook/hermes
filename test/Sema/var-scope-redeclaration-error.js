/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

function foo() {
  let x;
  {
    let y;
    var x;
    var y;
  }

  var a1;
  let a1;
}

{
  let z;
  var z;
}

function bar() {
    try {
        something();
    } catch ([e = 10]) {
        // Shadowing catch variable bound via destructuring is an error.
        var e = 10;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}var-scope-redeclaration-error.js:14:9: error: Identifier 'x' is already declared
// CHECK-NEXT:    var x;
// CHECK-NEXT:        ^
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:11:7: note: previous declaration
// CHECK-NEXT:  let x;
// CHECK-NEXT:      ^
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:15:9: error: Identifier 'y' is already declared
// CHECK-NEXT:    var y;
// CHECK-NEXT:        ^
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:13:9: note: previous declaration
// CHECK-NEXT:    let y;
// CHECK-NEXT:        ^
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:19:7: error: Identifier 'a1' is already declared
// CHECK-NEXT:  let a1;
// CHECK-NEXT:      ^~
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:18:7: note: previous declaration
// CHECK-NEXT:  var a1;
// CHECK-NEXT:      ^~
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:24:7: error: Identifier 'z' is already declared
// CHECK-NEXT:  var z;
// CHECK-NEXT:      ^
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:23:7: note: previous declaration
// CHECK-NEXT:  let z;
// CHECK-NEXT:      ^
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:32:13: error: Identifier 'e' is already declared
// CHECK-NEXT:        var e = 10;
// CHECK-NEXT:            ^
// CHECK-NEXT:{{.*}}var-scope-redeclaration-error.js:30:15: note: previous declaration
// CHECK-NEXT:    } catch ([e = 10]) {
// CHECK-NEXT:              ^
// CHECK-NEXT:Emitted 5 errors. exiting.
