/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

import * as a from "foo";
import b from "foo";
a;
b;

// CHECK-LABEL: import * as a@D0 from 'foo';
// CHECK-NEXT: import b@D1 from 'foo';
// CHECK-NEXT: a@D0;
// CHECK-NEXT: b@D1;
