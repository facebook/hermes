/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -dump-sema %s 2>&1) | %FileCheck %s --match-full-lines

function testStrict() {
  'use strict';
  // These redeclarations must error because they are in inner scopes.
  // Note that there's no function promotion because this is strict mode.
  {
    function a1() {}
    function a1() {}
// CHECK: {{.*}}: error: Identifier 'a1' is already declared

    var a2;
    function a2() {}
// CHECK: {{.*}}: error: Identifier 'a2' is already declared

    let a3;
    function a3() {}
// CHECK: {{.*}}: error: Identifier 'a3' is already declared

    try {} catch (a4) {
      function a4() {}
    }
// CHECK: {{.*}}: error: Identifier 'a4' is already declared
  }
}

function testLoose() {
  // Loose mode.
  {
    function b1() {}
    function b1() {}
// Fine in loose mode.
// CHECK-NOT: {{.*}}: error: Identifier 'b1' is already declared

    var b2;
    function b2() {}
// CHECK: {{.*}}: error: Identifier 'b2' is already declared

    let b3;
    function b3() {}
// CHECK: {{.*}}: error: Identifier 'b3' is already declared

    try {} catch (b4) {
      function b4() {}
    }
// CHECK: {{.*}}: error: Identifier 'b4' is already declared
  }
}

function testStrictTopLevel() {
  'use strict';
  function c1() {}
  function c1() {}
// Special case: this is fine at top-level.
// CHECK-NOT: {{.*}}: error: Identifier 'c1' is already declared

  var c2;
  function c2() {}
// Special case: this is fine at top-level.
// CHECK-NOT: {{.*}}: error: Identifier 'c2' is already declared

  let c3;
  function c3() {}
// CHECK: {{.*}}: error: Identifier 'c3' is already declared

  try {} catch (c4) {
    function c4() {}
  }
// CHECK: {{.*}}: error: Identifier 'c4' is already declared
}

function testLooseTopLevel() {
// Loose mode is the same as strict mode here.
  function d1() {}
  function d1() {}
// Special case: this is fine at top-level.
// CHECK-NOT: {{.*}}: error: Identifier 'd1' is already declared

  var d2;
  function d2() {}
// Special case: this is fine at top-level.
// CHECK-NOT: {{.*}}: error: Identifier 'd2' is already declared

  let d3;
  function d3() {}
// CHECK: {{.*}}: error: Identifier 'd3' is already declared

  try {} catch (d4) {
    function d4() {}
  }
// CHECK: {{.*}}: error: Identifier 'd4' is already declared
}
