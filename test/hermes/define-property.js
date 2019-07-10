// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

print("define-property");
//CHECK-LABEL: define-property

// Check sure that the accessor flag is not cleared when updating other flags.
var obj =  { get p() { return 42; }  };
Object.defineProperty(obj, 'p', {enumerable: false });
print(obj.p);
//CHECK-NEXT: 42
