/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -enable-hermes-internal=true %s | %FileCheck --match-full-lines --check-prefix=CHKHIE %s
// RUN: %hermes -enable-hermes-internal=false %s | %FileCheck --match-full-lines --check-prefix=CHKHID %s
// RUN: %hermes -Xhermes-internal-test-methods=true %s | %FileCheck --match-full-lines --check-prefix=CHKIME %s
// RUN: %hermes -Xhermes-internal-test-methods=false %s | %FileCheck --match-full-lines --check-prefix=CHKIMD %s

// Check that we can disable all fields of HermesInternal.
print(Object.getOwnPropertyNames(HermesInternal).length !== 0);
//CHKHIE: true
//CHKHID: false
//CHKIME: true
//CHKIMD: true

// Check that we can disable test methods in HermesInternal.
print(typeof HermesInternal.detachArrayBuffer);
//CHKHIE-NEXT: undefined
//CHKHID-NEXT: undefined
//CHKIME-NEXT: function
//CHKIMD-NEXT: undefined
