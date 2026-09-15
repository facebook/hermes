/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -Wx,-gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s

// Regression: `_sh_ljs_create_class` in StaticH.cpp held an open
// `GCScopeMarkerRAII` when it called `_sh_throw_current` on the
// `createClass` failure path. `_sh_throw_current` is `_sh_longjmp`,
// which bypasses C++ destructors — so the marker never flushed and the
// handles `createClass` had accumulated (e.g. the `Get(superclass,
// "prototype")` result) leaked into the unit's top `GCScope`. In a hot
// `try { class X extends Y {} } catch (e) {}` loop where `Y.prototype`
// is not an object or null, each iteration deposits a few unflushed
// slots; the unit scope eventually crosses
// `HERMESVM_DEBUG_MAX_GCSCOPE_HANDLES` (48) and aborts.
//
// Fix is structural (`marker.flush()` before `_sh_throw_current`).

'use strict';

function Y() {}
Y.prototype = 5; // not an object and not null → throws at class creation

function test() {
  try { class X extends Y {} } catch (e) {}
}

for (var i = 0; i < 50; ++i) test();

print('PASS');
// CHECK: PASS
