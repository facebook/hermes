/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser %s

// Make sure we can find the "program->body" section of the ESTree json file
// without crashing.

function f() { }
