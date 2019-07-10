// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -non-strict -target=HBC %s | %FileCheck --match-full-lines %s
// Make sure the global object has a prototype and prints as the correct class.

print(this);
//CHECK: [object global]
