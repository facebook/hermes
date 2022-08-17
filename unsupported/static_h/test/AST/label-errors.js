/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%hermes -hermes-parser %s 2>&1 || true) | %FileCheck %s --match-full-lines

break;
//CHECK: {{.*}}label-errors.js:10:1: error: 'break' not within a loop or a switch
//CHECK-NEXT: break;
//CHECK-NEXT: ^~~~~~

continue;
//CHECK: {{.*}}label-errors.js:15:1: error: 'continue' not within a loop
//CHECK-NEXT: continue;
//CHECK-NEXT: ^~~~~~~~~

label1: { continue label1; }
//CHECK: {{.*}}label-errors.js:20:20: error: continue label 'label1' is not a loop label
//CHECK-NEXT: label1: { continue label1; }
//CHECK-NEXT:                    ^~~~~~
//CHECK: {{.*}}label-errors.js:20:1: note: label defined here
//CHECK-NEXT: label1: { continue label1; }
//CHECK-NEXT: ^~~~~~

label2: { label2: ; }
//CHECK: {{.*}}label-errors.js:28:11: error: label 'label2' is already defined
//CHECK-NEXT: label2: { label2: ; }
//CHECK-NEXT:           ^~~~~~
//CHECK: {{.*}}label-errors.js:28:1: note: previous definition
//CHECK-NEXT: label2: { label2: ; }
//CHECK-NEXT: ^~~~~~

label3: label3: ;
//CHECK: {{.*}}label-errors.js:36:9: error: label 'label3' is already defined
//CHECK-NEXT: label3: label3: ;
//CHECK-NEXT:         ^~~~~~
//CHECK: {{.*}}label-errors.js:36:1: note: previous definition
//CHECK-NEXT: label3: label3: ;
//CHECK-NEXT: ^~~~~~

break label1;
//CHECK: {{.*}}label-errors.js:44:7: error: label 'label1' is not defined
//CHECK-NEXT: break label1;
//CHECK-NEXT:       ^~~~~~

label1:; // No error here
