/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s

// Per spec §7.4.13 AsyncIteratorClose:
//   4. Let innerResult be GetMethod(iterator, "return").
//   5. If innerResult.[[Type]] is normal, then ...
//   6. If completion.[[Type]] is throw, return Completion(completion).
//   7. If innerResult.[[Type]] is throw, return Completion(innerResult).
//
// When the surrounding completion is a throw, an exception thrown by
// GetMethod (e.g. a `return` accessor that throws, or a non-callable
// `return` value tripping IsCallable's TypeError) must be suppressed
// — the original throw wins. IRGen wraps the GetMethod call inside
// the try/catch only on the ignoreInnerException path; this test
// exercises that path via for-await-of body throws.

// Case 1: `return` accessor throws.
async function case1() {
  var asyncIterable = {
    [Symbol.asyncIterator]() {
      return {
        next() { return { done: false, value: null }; },
        get return() { throw { name: 'inner-getter-error' }; },
      };
    },
  };
  try {
    for await (var _ of asyncIterable) {
      throw 'body-error';
    }
  } catch (e) {
    print('case1:', e);
  }
}

// Case 2: `return` is non-callable (a plain value).
async function case2() {
  var asyncIterable = {
    [Symbol.asyncIterator]() {
      return {
        next() { return { done: false, value: null }; },
        return: true,
      };
    },
  };
  try {
    for await (var _ of asyncIterable) {
      throw 'body-error';
    }
  } catch (e) {
    print('case2:', e);
  }
}

case1();
case2();
// CHECK: case1: body-error
// CHECK-NEXT: case2: body-error
