/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods=true %s | %FileCheck --match-full-lines --check-prefix=CHKIME %s
// RUN: %hermes -Xhermes-internal-test-methods=false %s | %FileCheck --match-full-lines --check-prefix=CHKIMD %s

// Check that we can disable test methods in HermesInternal.

print(typeof HermesInternal.detachArrayBuffer);
//CHKIME: function
//CHKIMD: undefined
