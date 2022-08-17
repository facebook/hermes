/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -enable-hermes-internal=true %s | %FileCheck --match-full-lines --check-prefix=CHKHIE %s
// RUN: %hermes -enable-hermes-internal=false %s | %FileCheck --match-full-lines --check-prefix=CHKHID %s
// RUN: %hermes -Xhermes-internal-test-methods=true %s | %FileCheck --match-full-lines --check-prefix=CHKIME %s
// RUN: %hermes -Xhermes-internal-test-methods=false %s | %FileCheck --match-full-lines --check-prefix=CHKIMD %s

// concat
// hasPromise
// setPromiseRejectionTrackingHook
// enablePromiseRejectionTracker
// enqueueJob
// useEngineQueue
var SAFE_FIELDS_COUNT = 6;

// Check that we can disable unsafe fields of HermesInternal.
print(Object.getOwnPropertyNames(HermesInternal).length)
print(Object.getOwnPropertyNames(HermesInternal).length !== SAFE_FIELDS_COUNT);
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

// Check that safe fields such as HermesInternal.concat is
// kept around even when HermesInternal is diabled.
print(typeof HermesInternal.concat)
//CHKHIE-NEXT: function
//CHKHID-NEXT: function
//CHKIME-NEXT: function
//CHKIMD-NEXT: function
print(`hello${1 + 1}world`);
//CHKHIE-NEXT: hello2world
//CHKHID-NEXT: hello2world
//CHKIME-NEXT: hello2world
//CHKIMD-NEXT: hello2world
