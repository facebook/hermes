// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -serializevm-path=%t -target=HBC %s
// RUN: %hermes -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

var xx = {};
xx.ascii1 = 1;
xx.ascii2 = 2;
xx.unicodë3 = 3;
xx.unicodë4 = 4;

serializeVM(function () {
    print(xx.ascii1);
// CHECK: 1
    print(xx.ascii2);
// CHECK: 2
    print(xx.unicodë3);
// CHECK: 3
    print(xx.unicodë4);
// CHECK: 4
})
