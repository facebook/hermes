/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s
// Make sure we generate code that successfully verifies

function foo(n, r) {
    switch (n && r) {}
}
