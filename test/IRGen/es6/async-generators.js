/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc %s --dump-bytecode 2>&1) | %FileCheck %s

async function* f() {};
// CHECK: async generators are unsupported
