/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O -gc-sanitize-handles=1 %s
// This is an empty file, just used to ensure that the Runtime can be
// constructed with full handle sanitization turned on and not crash.
