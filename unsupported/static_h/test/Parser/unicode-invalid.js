/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s 2>&1 ) | %FileCheck %s

// Verify that invalid Unicode identifiers cause errors
// to be emitted.

 ͛xyz = false;
// Starts with a combining mark
// CHECK: error: unrecognized Unicode character \u35b
// CHECK-NEXT:  ͛xyz = false;

᥊abc = false;
// Starts with a digit
// CHECK: error: unrecognized Unicode character \u194a
// CHECK-NEXT: ᥊abc = false;

͋de = false;
// Starts with a digit with combining mark
// CHECK: error: unrecognized Unicode character \u34b
// CHECK-NEXT:  ͋de = false;

͓fg = false;
// Starts with connector puncutation
// CHECK: error: unrecognized Unicode character \u353
// CHECK-NEXT:  ͓fg = false;
