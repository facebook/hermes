/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --prompt="" < %s | %FileCheck %s

// loadSegment() shouldn't crash in the REPL
try {
    loadSegment()
} catch (e) {
    print("Exception");
}
// CHECK: Exception
