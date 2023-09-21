/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// We shouldn't check-in a file with a name containing unicode. So instead,
// we create the file with the unicode character, containing the contents
// of this test.
// RUN: cp %s %t.unicÓde.js && %shermes %t.unicÓde.js -g1 -exec | %FileCheck --match-full-lines %s
"use strict";

// This filename has a unicode character in it. Make sure that
// this unicode filename is properly printed into the Error.stack
// property.

print((new Error).stack.includes("Ó"));
//CHECK: true
