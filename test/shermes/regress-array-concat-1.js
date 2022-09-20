/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

var a = [];
a.__defineGetter__(0, function () {});
// Convert the result to string and print it
print(a.concat([])[0] + "foo");
//CHECK: undefinedfoo
