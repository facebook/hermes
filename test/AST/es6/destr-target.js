/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%hermes -hermes-parser %s 2>&1 || true) | %FileCheck %s --match-full-lines

({a : 0} = x)
//CHECK: {{.*}}destr-target.js:10:7: error: invalid assignment left-hand side
//CHECK-NEXT: ({a : 0} = x)
//CHECK-NEXT:       ^

({a : function a() {} } = x)
//CHECK: {{.*}}destr-target.js:15:7: error: invalid assignment left-hand side
//CHECK-NEXT: ({a : function a() {} } = x)
//CHECK-NEXT:       ^~~~~~~~~~~~~~~
