/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// REQUIRES: windows
// RUN: (CC=cmd %shermes -v -o %t.exe %s || true) 2>&1 | %FileCheck %s

// CHECK: cmd.exe
// CHECK-NOT: {{(^| )-IC( |$)}}
// CHECK-NOT: {{(^| )-LC( |$)}}
// CHECK-NOT: {{(^| )-Wl,-rpath C( |$)}}
// CHECK-NOT: {{[A-Za-z]:[^ ]*:[A-Za-z]:}}
// CHECK: {{(^| )-o }}

print("ok");
