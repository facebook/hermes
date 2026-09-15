/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xjit -Xjit-threshold=1 -Xjit-crash-on-error -Xjit-hc-id-limit=0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -fno-inline -Xjit -Xjit-threshold=1 -Xjit-crash-on-error -Xjit-hc-id-limit=0 -Xdump-jitcode=3 %s 2>&1 | %FileCheck --match-full-lines --check-prefix=JITCODE %s
// REQUIRES: jit

// When the lazy JIT id space is exhausted, initHCLazyIDMayAlloc returns 0 and
// the GetById specializations cannot be emitted. The site must then fall back
// to the generic inline cache, rather than compiling with no cache at all.
//
// -Xjit-hc-id-limit makes this reachable without interning 65535 hidden
// classes. A limit of 0 exhausts the space before the first assignment, so
// every specialization bails deterministically.

// Each of these reads a property off the prototype, which is the shape that
// selects parent specialization -- the tier that returns early and, before
// the fall-back, left the site with no inline cache at all.
function getA(o) {
  return o.pa;
}
function getB(o) {
  return o.pb;
}
function getC(o) {
  return o.pc;
}
function getD(o) {
  return o.pd;
}

function make(name, val) {
  var proto = {};
  proto[name] = val;
  var o = {};
  o["own_" + name] = 1;
  Object.setPrototypeOf(o, proto);
  return o;
}

var oa = make("pa", 10);
var ob = make("pb", 20);
var oc = make("pc", 30);
var od = make("pd", 40);

// Warm each site so its cache entry is a monomorphic negMatch, then keep
// calling so every function crosses the JIT threshold.
var sum = 0;
for (var i = 0; i < 20; ++i) {
  sum += getA(oa);
  sum += getB(ob);
  sum += getC(oc);
  sum += getD(od);
}

print(sum);
// CHECK: 2000

// Values must still be correct for classes that never received an id.
print(getA(oa), getB(ob), getC(oc), getD(od));
// CHECK-NEXT: 10 20 30 40

// Reading through a different shape must still work, exercising the generic
// cache's miss path at a site whose specialization could not be emitted.
print(getA(make("pa", 11)), getD(make("pd", 44)));
// CHECK-NEXT: 11 44

// The generic tier must be present in each compiled function. Without the
// fall-back these functions emit neither a specialization nor a property
// cache: the fast path just runs off into the slow-path call, and the
// "Read property cache" marker is absent between the function label and its
// completion message.
// JITCODE: getA:
// JITCODE: // Read property cache
// JITCODE: JIT successfully compiled FunctionID 1, 'getA'
// JITCODE: getB:
// JITCODE: // Read property cache
// JITCODE: JIT successfully compiled FunctionID 2, 'getB'
// JITCODE: getC:
// JITCODE: // Read property cache
// JITCODE: JIT successfully compiled FunctionID 3, 'getC'
// JITCODE: getD:
// JITCODE: // Read property cache
// JITCODE: JIT successfully compiled FunctionID 4, 'getD'
