// RUN: %hermesc -O -fstatic-require -commonjs -emit-binary -out=/dev/null %s 2>&1 | %FileCheck --match-full-lines %s

foo(require);
//CHECK: {{.*}}cjs-static-fail-2.js:3:4: warning: 'require' used as function call argument
//CHECK-NEXT: foo(require);
//CHECK-NEXT:    ^

require(require);
//CHECK: {{.*}}cjs-static-fail-2.js:8:8: warning: 'require' used as function call argument
//CHECK-NEXT: require(require);
//CHECK-NEXT:        ^

new require("a");
//CHECK: {{.*}}cjs-static-fail-2.js:13:12: warning: 'require' used as a constructor
//CHECK-NEXT: new require("a");
//CHECK-NEXT:            ^

this.prop1 = require;
//CHECK: {{.*}}cjs-static-fail-2.js:18:12: warning: 'require' escapes or is modified
//CHECK-NEXT: this.prop1 = require;
//CHECK-NEXT:            ^

require();
//CHECK: {{.*}}cjs-static-fail-2.js:23:8: warning: require() invoked without arguments
//CHECK-NEXT: require();
//CHECK-NEXT:        ^

require("b", "c");
//CHECK: {{.*}}cjs-static-fail-2.js:28:8: warning: Additional require() arguments will be ignored
//CHECK-NEXT: require("b", "c");
//CHECK-NEXT:        ^

require(foo + 1);
//CHECK: {{.*}}cjs-static-fail-2.js:33:8: warning: require() argument cannot be coerced to constant string at compile time
//CHECK-NEXT: require(foo + 1);
//CHECK-NEXT:        ^
//CHECK: {{.*}}cjs-static-fail-2.js:33:9: note: First argument of require()
//CHECK-NEXT: require(foo + 1);
//CHECK-NEXT:         ^

function func() {
    require("c");
    var r = require;
}

exports.func = func;
