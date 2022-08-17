// Remove this once the REPL can handle block comments.
// @lint-ignore-every LICENSELINT

// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

// RUN: cat %s | %hermes -prompt="" | %FileCheck --match-full-lines %s

"wrapped eval"
// CHECK-LABEL: "wrapped eval"
{a:1}
// CHECK-NEXT: { a: 1 }
{if (true) {1} else {2}}
// CHECK-NEXT: 1
{if (false) {1} else {2}}
// CHECK-NEXT: 2
