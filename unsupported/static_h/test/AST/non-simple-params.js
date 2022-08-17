/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s 2>&1 ) | %FileCheck --match-full-lines %s

function foo(aa = 1, bbb, [{c, aa}, bbb]) {}
//CHECK: {{.*}}non-simple-params.js:10:32: error: cannot declare two parameters with the same name 'aa'
//CHECK-NEXT: function foo(aa = 1, bbb, [{c, aa}, bbb]) {}
//CHECK-NEXT:                                ^~
//CHECK: {{.*}}non-simple-params.js:10:37: error: cannot declare two parameters with the same name 'bbb'
//CHECK-NEXT: function foo(aa = 1, bbb, [{c, aa}, bbb]) {}
//CHECK-NEXT:                                     ^~~

function bar(p = 10) {
    "use asm";
    "use strict";
//CHECK: {{.*}}non-simple-params.js:20:5: error: 'use strict' not allowed inside function with non-simple parameter list
//CHECK-NEXT:     "use strict";
//CHECK-NEXT:     ^~~~~~~~~~~~~

}

function baz(f = function() {}) {
    "use strict";
//CHECK: {{.*}}non-simple-params.js:28:5: error: 'use strict' not allowed inside function with non-simple parameter list
//CHECK-NEXT:     "use strict";
//CHECK-NEXT:     ^~~~~~~~~~~~~

}
