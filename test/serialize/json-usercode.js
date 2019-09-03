// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

// Call some JSON functions just to see if "JSON" is still there.

// JSON constructor is not callable. Serialize directly.
serializeVM(function () {
  print(JSON.toString());
  //CHECK-LABEL: [object JSON]

  // JSON.parse()

  print(JSON.parse("5.6") === 5.6);
  //CHECK-NEXT: true

  print(JSON.parse("true") === true);
  //CHECK-NEXT: true

  print(JSON.parse("false") === false);
  //CHECK-NEXT: true

  print(JSON.parse("null") === null);
  //CHECK-NEXT: true

  print(JSON.parse("\"str\"") === "str");
  //CHECK-NEXT: true
})
