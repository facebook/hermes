// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

function *badParam(x = yield) {}
// CHECK: {{.*}}:8:24: error: 'yield' not allowed in a formal parameter
// CHECK: function *badParam(x = yield) {}
// CHECK:                        ^~~~~

({ *gen(x = yield) {} })
// CHECK: {{.*}}:13:13: error: 'yield' not allowed in a formal parameter
// CHECK: ({ *gen(x = yield) {} })
// CHECK:             ^~~~~

function *badParam2(x = function*(y = yield) {}) {}
// CHECK: {{.*}}:18:39: error: 'yield' not allowed in a formal parameter
// CHECK: function *badParam2(x = function*(y = yield) {}) {}
// CHECK:                                       ^~~~~
