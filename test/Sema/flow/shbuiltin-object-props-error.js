/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -typed -dump-sema %s 2>&1) | %FileCheck --match-full-lines %s

$SHBuiltin.objectKeys();
// CHECK: {{.*}}shbuiltin-object-props-error.js:[[@LINE-1]]:1: error: ft: objectKeys requires exactly one argument

$SHBuiltin.objectValues({}, {});
// CHECK: {{.*}}shbuiltin-object-props-error.js:[[@LINE-1]]:1: error: ft: objectValues requires exactly one argument

$SHBuiltin.objectEntries();
// CHECK: {{.*}}shbuiltin-object-props-error.js:[[@LINE-1]]:1: error: ft: objectEntries requires exactly one argument
