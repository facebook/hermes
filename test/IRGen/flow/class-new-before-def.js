/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed  %s 2>&1 ) | %FileCheck --match-full-lines %s
// RUN: (! %shermes -typed -Xenable-tdz %s 2>&1 ) | %FileCheck --match-full-lines --check-prefix=CHKTDZ %s

'use strict';

(function main() {
    new C(10);
    class C {
        x: number;
        constructor(x) {
            this.x = x;
        }
    }
})();

//CHECK: {{.*}}class-new-before-def.js:14:9: error: Cannot construct class before its definition.

//CHKTDZ: {{.*}}class-new-before-def.js:14:9: error: TDZ violation: reading from uninitialized variable 'C'
//CHKTDZ: {{.*}}class-new-before-def.js:14:9: error: Cannot construct class before its definition.
