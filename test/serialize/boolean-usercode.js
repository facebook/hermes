// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict"

var t = new Boolean(true)
var f = new Boolean(false)

serializeVM(function() {
  print('Boolean');
  // CHECK-LABEL: Boolean
  print(t, f);
  // CHECK-NEXT: true false
})
