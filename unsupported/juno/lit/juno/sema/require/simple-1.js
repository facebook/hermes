/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s %S/simple-2.js %S/simple-3.js | %FileCheck %s --match-full-lines

import 'simple-3';

require('./simple-2.js');
require('simple-3');

// CHECK-LABEL: Module: {{.*}}/simple-1.js
// CHECK: 3 require resolutions
