/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s

// Test that the variables inside "finally" blocks don't crash the compiler,
// since they have to be declared twice when we inline the finally
// block for the normal and exceptional paths.

try{
} finally {
  let v1 = 10;
  try {
  } catch(v2){
      let v3 = 30;
  }
}

