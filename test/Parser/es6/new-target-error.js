// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermesc -Xflow-parser -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

new.target;
// CHECK: {{.*}}:8:1: error: 'new.target' not in a function
// CHECK: new.target;
// CHECK: ^~~~~~~~~~
