/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s

// For readability, symbols in emitted C code are annotated with a C comment
// containing the corresponding string literal. If the literal happens to
// contain the characters "*/", it causes a C compilation error.
// Make sure this is properly escaped.

print("/**/");
