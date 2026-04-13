/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -Wx,-Xmicrotask-queue %s | %FileCheck --match-full-lines %s

// Test that cleanup() correctly handles ArrayStorage reallocation caused by
// the cleanup callback calling register(). Regression test for T264548637.
//
// The core bug: cleanup() caches the ArrayStorage pointer (lv.cells) before
// the loop. If a callback calls register() and triggers reallocation, the
// loop continues reading from the stale old storage (S1) while mutations
// (free(), numUsedCells_--) operate on the new storage (S2) via self->cells_.
// This divergence is always incorrect regardless of whether unregister() is
// also called, because the loop's view of the data (S1) no longer matches
// the actual storage being modified (S2).
//
// When the callback also calls unregister(), the bug becomes exploitable:
// slots already freed in S2 by unregister() still appear as live objects in
// stale S1, causing double-free and double-decrement of numUsedCells_.
// This corrupts numUsedCells_ to a small value, causing tryShrink() to
// allocate an undersized buffer that overflows in pushWithinCapacity().
//
// Detailed flow (with N=10 targets, P=10 padding registrations):
//
// 1. cleanup() caches lv.cells pointing to storage S1 (size 10).
// 2. Index 0: dead target -> free(0), numUsedCells_ = 9. Callback fires.
// 3. Callback: register() x10 -> push_back causes reallocation S1->S2.
//    self->cells_ now points to S2. numUsedCells_ = 19.
//    unregister(sharedToken) frees indices 1-9 in S2. numUsedCells_ = 10.
// 4. Loop resumes on stale S1 where indices 1-9 still look like objects.
//    Each iteration: free() in S2 (double-free) + numUsedCells_-- (double-
//    decrement). After 9 iterations: numUsedCells_ = 10 - 9 = 1.
// 5. tryShrink(): occupancy = 1/19 = 0.05 < kOccupancyRatio (0.25).
//    newSize = max(1*2, 4) = 4. But 10 records are alive -> overflow when
//    pushWithinCapacity() copies them into the capacity-4 buffer.
//
// The values N and P must satisfy: P - (N-1) is small enough that
// (P - (N-1)) / (N + P - 1) < kOccupancyRatio (0.25), so that tryShrink
// fires and allocates an undersized buffer. N=P=10 gives occupancy ~5%.

var triggered = false;
var sharedToken = {};

var fr = new FinalizationRegistry(function cleanup(heldValue) {
    if (triggered) return;
    triggered = true;

    // Force ArrayStorage reallocation by registering new objects.
    var paddingToken = {};
    for (var i = 0; i < 10; i++) {
        fr.register({}, "padding-" + i, paddingToken);
    }

    // Unregister the original records in the (now reallocated) storage.
    fr.unregister(sharedToken);
});

// Register 10 target objects that will become unreachable.
(function registerTargets() {
    for (var i = 0; i < 10; i++) {
        var target = {};
        fr.register(target, "victim-" + i, sharedToken);
    }
})();

// Force GC to collect the targets and trigger cleanup callbacks.
gc();

print("done");
// CHECK: done
