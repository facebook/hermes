/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s | %FileCheck --match-full-lines %s

print('Internal Binding');
// CHECK-LABEL: Internal Binding

var _internalBinding = internalBinding('constants'),
  constants = _internalBinding.fs;

var F_OK = constants.F_OK,
  R_OK = constants.R_OK,
  W_OK = constants.W_OK,
  X_OK = constants.X_OK,
  O_WRONLY = constants.O_WRONLY,
  O_SYMLINK = constants.O_SYMLINK;

if (F_OK != undefined) {
  print(F_OK);
} else print(0);
// CHECK-NEXT: 0
if (R_OK != undefined) {
  print(R_OK);
} else print(4);
// CHECK-NEXT: 4
if (W_OK != undefined) {
  print(W_OK);
} else print(2);
// CHECK-NEXT: 2
if (X_OK != undefined) {
  print(X_OK);
} else print(1);
// CHECK-NEXT: 1
if (O_WRONLY != undefined) {
  print(O_WRONLY);
} else print(1);
// CHECK-NEXT: 1
