/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1 ) | %FileCheck --match-full-lines %s
'use strict';

class let {}
// CHECK: {{.*}}:11:7: error: Invalid use of strict mode reserved word as binding identifier
// CHECK: class let {}
// CHECK:       ^~~

class static {}
// CHECK: {{.*}}:16:7: error: 'identifier' expected after 'class'
// CHECK: class static {}
// CHECK: ~~~~~~^
