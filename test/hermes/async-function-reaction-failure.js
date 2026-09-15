/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

// The rejection tracker installs hooks that run inside the polyfill's
// `handle()` and `reject()`. Those hooks call host timer functions, so a
// hostile host can make the Promise machinery throw at points the async
// function driver has to survive.

HermesInternal.enablePromiseRejectionTracker({
  allRejections: true,
  onUnhandled: function (id, error) {
    print('unhandled', error.message);
  },
});

// Attaching an await's reaction runs the tracker's "handled" hook. If that
// throws, nothing will ever resume the generator, so it must be closed for
// its `finally` blocks to run, and the result Promise must be rejected.
var tracked = Promise.reject(new Error('tracked'));
var realClearTimeout = clearTimeout;

async function attachFails() {
  try {
    await tracked;
    print('unreachable resume');
  } finally {
    print('finally ran');
  }
}

var attachResult;
clearTimeout = function () {
  throw new Error('attach failure');
};
try {
  attachResult = attachFails();
} finally {
  clearTimeout = realClearTimeout;
}
// CHECK: finally ran
attachResult.then(
  function (value) {
    print('attach resolved', value);
  },
  function (error) {
    print('attach rejected', error.message);
  }
);

// Rejecting the result Promise runs the tracker's "rejected" hook, which
// arms a timer. If that throws after the rejection has been recorded, the
// driver's own error path must not settle the Promise a second time —
// re-running the hook would let the throw escape spawn() into the caller.
var realSetTimeout = setTimeout;

async function settleThenThrow() {
  throw new Error('original');
}

var settleResult = null;
var escaped = null;
setTimeout = function () {
  throw new Error('hook failure');
};
try {
  settleResult = settleThenThrow();
} catch (e) {
  escaped = e;
} finally {
  setTimeout = realSetTimeout;
}
print('escaped into caller', escaped === null ? 'no' : escaped.message);
// CHECK-NEXT: escaped into caller no
if (settleResult === null) {
  print('settle produced no promise');
} else {
  settleResult.then(
    function (value) {
      print('settle resolved', value);
    },
    function (error) {
      print('settle rejected', error.message);
    }
  );
}
// CHECK-NEXT: attach rejected attach failure
// CHECK-NEXT: settle rejected original

// The tracker still reports an async function's own unhandled rejection.
async function neverHandled() {
  await 1;
  throw new Error('async unhandled');
}
neverHandled();
// CHECK-NEXT: unhandled tracked
// CHECK-NEXT: unhandled async unhandled
