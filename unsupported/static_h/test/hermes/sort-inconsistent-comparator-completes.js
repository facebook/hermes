/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// Make sure the sort completes even with an inconsistent comparator.
[1, 1, 1, 1, 1, 1, 1].sort(function(a, b) { 
  if (a === 1) return -1; 
  else if (b === 1) return 1; 

  return 0;
})
print("DONE");
//CHECK: DONE
