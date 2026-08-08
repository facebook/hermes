/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// REQUIRES: windows
// RUN: (%shermes -v -Wc,--shermes-invalid-cc-option -o %t.exe %s || true) 2>&1 | %FileCheck %s

// CHECK: --shermes-invalid-cc-option
// CHECK-NOT: {{(^| )-I[A-Za-z]( |$)}}
// CHECK-NOT: {{(^| )-L[A-Za-z]( |$)}}
// CHECK-NOT: {{(^| )-Wl,-rpath [A-Za-z]( |$)}}
// CHECK-NOT: {{[A-Za-z]:[^ ]*:[A-Za-z]:}}
// CHECK: {{(^| )-o }}

print("ok");
