// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

function foo(aa = 1, bbb, [{c, aa}, bbb]) {}
//CHECK: {{.*}}non-simple-params.js:3:32: error: cannot declare two parameters with the same name 'aa'
//CHECK-NEXT: function foo(aa = 1, bbb, [{c, aa}, bbb]) {}
//CHECK-NEXT:                                ^~
//CHECK: {{.*}}non-simple-params.js:3:37: error: cannot declare two parameters with the same name 'bbb'
//CHECK-NEXT: function foo(aa = 1, bbb, [{c, aa}, bbb]) {}
//CHECK-NEXT:                                     ^~~

function bar(p = 10) {
    "use asm";
    "use strict";
//CHECK: {{.*}}non-simple-params.js:13:5: error: 'use strict' not allowed inside function with non-simple parameter list
//CHECK-NEXT:     "use strict";
//CHECK-NEXT:     ^~~~~~~~~~~~~

}

function baz(f = function() {}) {
    "use strict";
//CHECK: {{.*}}non-simple-params.js:21:5: error: 'use strict' not allowed inside function with non-simple parameter list
//CHECK-NEXT:     "use strict";
//CHECK-NEXT:     ^~~~~~~~~~~~~

}
