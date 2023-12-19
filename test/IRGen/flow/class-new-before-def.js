/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -typed  %s 2>&1 ) | %FileCheck --match-full-lines %s

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

//CHECK: {{.*}}class-new-before-def.js:13:9: error: Cannot construct class before its definition.

