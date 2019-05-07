// RUN: (! %hermes -non-strict %s 2>&1 ) | %FileCheck --match-full-lines %s

// Make sure we scan directive prologues before doing everything else.

function f1() {
    "use strict" // comment
     delete x;
//CHECK: {{.*}}directives-3.js:7:6: error: 'delete' of a variable is not allowed in strict mode
//CHECK-NEXT:      delete x;
//CHECK-NEXT:      ^~~~~~~~

}

function f2() {
    "use strict" /* comment */ ;
     delete x;
//CHECK: {{.*}}directives-3.js:16:6: error: 'delete' of a variable is not allowed in strict mode
//CHECK-NEXT:      delete x;
//CHECK-NEXT:      ^~~~~~~~

}

function f3() {
    "use strict" /* comment */
     delete x;
//CHECK: {{.*}}directives-3.js:25:6: error: 'delete' of a variable is not allowed in strict mode
//CHECK-NEXT:      delete x;
//CHECK-NEXT:      ^~~~~~~~

}

function f4(eval) {
//CHECK: {{.*}}directives-3.js:32:13: error: cannot declare 'eval'
//CHECK-NEXT: function f4(eval) {
//CHECK-NEXT:             ^~~~

    "use strict"
}

function f5(eval) {"use strict"  }
//CHECK: {{.*}}directives-3.js:40:13: error: cannot declare 'eval'
//CHECK-NEXT: function f5(eval) {"use strict"  }
//CHECK-NEXT:             ^~~~
