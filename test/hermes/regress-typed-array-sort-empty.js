/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s

// Sorting a zero-length TypedArray without a compareFn must not pass a null
// pointer to qsort. This triggers a UBSan error when the backing ArrayBuffer
// has a null data block.

new Uint8Array(0).sort();
new Int8Array(0).sort();
new Float64Array(0).sort();
