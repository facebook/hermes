/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Wno-direct-eval %s | %FileCheck --match-full-lines %s
"use strict";

print('Parser');
// CHECK-LABEL: Parser
function makeDeep(d) {
    return d == 0 ? "" : "function f" + d + "() {" + makeDeep(d - 1) + "}";
}
try {
  new Function(makeDeep(1030));
} catch (e) {
    print('caught', e.name, e.message);
}
// CHECK-NEXT: caught SyntaxError{{.*}}Too many nested{{.*}}
