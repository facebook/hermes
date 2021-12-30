/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -non-strict %s 2>&1 ) | %FileCheck --match-full-lines %s

// Make sure we scan directive prologues before doing everything else.

function f1() {
    "use strict" // comment
     delete x;
//CHECK: {{.*}}directives-3.js:14:6: error: 'delete' of a variable is not allowed in strict mode
//CHECK-NEXT:      delete x;
//CHECK-NEXT:      ^~~~~~~~

}

function f2() {
    "use strict" /* comment */ ;
     delete x;
//CHECK: {{.*}}directives-3.js:23:6: error: 'delete' of a variable is not allowed in strict mode
//CHECK-NEXT:      delete x;
//CHECK-NEXT:      ^~~~~~~~

}

function f3() {
    "use strict" /* comment */
     delete x;
//CHECK: {{.*}}directives-3.js:32:6: error: 'delete' of a variable is not allowed in strict mode
//CHECK-NEXT:      delete x;
//CHECK-NEXT:      ^~~~~~~~

}

function f4(eval) {
//CHECK: {{.*}}directives-3.js:39:13: error: cannot declare 'eval'
//CHECK-NEXT: function f4(eval) {
//CHECK-NEXT:             ^~~~

    "use strict"
}

function f5(eval) {"use strict"  }
//CHECK: {{.*}}directives-3.js:47:13: error: cannot declare 'eval'
//CHECK-NEXT: function f5(eval) {"use strict"  }
//CHECK-NEXT:             ^~~~
