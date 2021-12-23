/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s %S/simple-2.js | %FileCheck %s --match-full-lines

require('./simple-2.js');

// CHECK-LABEL: Module: {{.*}}/simple-1.js
// CHECK: 1 require resolutions
