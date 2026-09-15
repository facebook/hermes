/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xes6-proxy -gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -Wx,-Xes6-proxy,-gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s

// Regression: in `putByIndex_RJS`, `_sh_throw_current` (which is
// `_sh_longjmp`) was called while a `GCScopeMarkerRAII` was still in
// scope. longjmp bypasses C++ destructors, so the marker never flushed,
// and the handles it had accumulated leaked into the unit's top
// `GCScope`. In a hot `try { ... } catch(e) {}` loop, each throwing
// iteration deposited a few unflushed slots; after ~10 iterations the
// unit scope crossed `HERMESVM_DEBUG_MAX_GCSCOPE_HANDLES` (48) and the
// next handle allocation aborted.
//
// The repro is one strict-mode indexed write to a non-extensible
// receiver whose [[Prototype]] is a TypedArray. The write throws
// `TypeError`, exercising the leaking throw path. `-gc-sanitize-handles
// =1` keeps the abort deterministic.
//
// Fix is structural (`marker.flush()` before `_sh_throw_current`), so
// the test passes regardless of iteration count; 50 leaves plenty of
// margin (~5x the buggy threshold) without dominating test time.

'use strict';

function test() {
  var t = new Int8Array([0]);
  var r = Object.preventExtensions(Object.create(t));
  try { r[0] = 1; } catch (e) {}
}

for (var i = 0; i < 50; ++i) test();

print('PASS');
// CHECK: PASS
