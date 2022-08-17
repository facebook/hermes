/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -max-diagnostic-width 80 -hermes-parser -dump-ir %s) 2>&1 | %FileCheck %s --match-full-lines --strict-whitespace

0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 42invalid;
//CHECK:{{.*}}max_diagnostic_width.js:10:89: error: invalid numeric literal
//CHECK-NEXT:... 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 42invalid;
//CHECK-NEXT:                                    ^~~~~~~~~

42invalid + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0;
//CHECK:{{.*}}max_diagnostic_width.js:15:1: error: invalid numeric literal
//CHECK-NEXT:42invalid + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0...
//CHECK-NEXT:^~~~~~~~~

0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 42invalid + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0;
//CHECK:{{.*}}max_diagnostic_width.js:20:49: error: invalid numeric literal
//CHECK-NEXT:... 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 42invalid + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0...
//CHECK-NEXT:                                    ^~~~~~~~~

42longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong
//CHECK:{{.*}}max_diagnostic_width.js:25:1: error: invalid numeric literal
//CHECK-NEXT:42longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong
//CHECK-NEXT:^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


"😺" + 42invalid + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0;
//CHECK:{{.*}}max_diagnostic_width.js:31:10: error: invalid numeric literal
//CHECK-NEXT:"😺" + 42invalid + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 +...

"😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺" + 42invalid;
//CHECK:{{.*}}max_diagnostic_width.js:35:{{[0-9]+}}: error: invalid numeric literal
//CHECK-NEXT:...😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺😺" + 42invalid;

0 +  42"	" + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0;
//CHECK:{{.*}}max_diagnostic_width.js:39:8: error: ';' expected
//CHECK-NEXT:0 +  42"        " + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0 + 0...
