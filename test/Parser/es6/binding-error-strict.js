// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermesc %s 2>&1 ) | %FileCheck --match-full-lines %s
'use strict';

class let {}
// CHECK: {{.*}}:9:7: error: Invalid use of strict mode reserved word as binding identifier
// CHECK: class let {}
// CHECK:       ^~~

class static {}
// CHECK: {{.*}}:14:7: error: 'identifier' expected in class declaration
// CHECK: class static {}
// CHECK: ~~~~~~^
