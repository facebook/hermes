/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Any pass that runs once is a good candidate to be used here.

// RUN: %hermesc -O --dump-bytecode %s --Xdump-after-all 2>&1 | %FileCheck --check-prefix AFTER-ALL %s
// RUN: %hermesc -O --dump-bytecode %s --Xdump-after Auditor 2>&1 | %FileCheck --check-prefix AFTER-PASS %s
// RUN: %hermesc -O --dump-bytecode %s --Xdump-before-all 2>&1 | %FileCheck --check-prefix BEFORE-ALL %s
// RUN: %hermesc -O --dump-bytecode %s --Xdump-before Auditor 2>&1 | %FileCheck --check-prefix BEFORE-PASS %s

function firstFunction() { print(10); }

function _2ndFunction() { print(20); }

// When --Xdump-before-all is specified we shouldn't see any *** AFTER lines
// BEFORE-ALL-NOT: *** AFTER {{Module|Function}} pass

// When --Xdump-after-all is specified we shouldn't see any *** BEFORE lines
// AFTER-ALL-NOT: *** BEFORE {{Module|Function}} pass

// Should see the *** AFTER line for the requested pass, and nothing else after
// that.
// AFTER-PASS: *** AFTER Function pass Auditor
// AFTER-PASS: *** AFTER Function pass Auditor
// AFTER-PASS: *** AFTER Function pass Auditor
// AFTER-PASS-NOT: ***

// Should see the *** BEFORE line for the requested pass, and nothing else after
// that.
// BEFORE-PASS: *** BEFORE Function pass Auditor
// BEFORE-PASS: *** BEFORE Function pass Auditor
// BEFORE-PASS: *** BEFORE Function pass Auditor
// BEFORE-PASS-NOT: ***
