/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -g %s \
// RUN:   | %FileCheck --match-full-lines %s -check-prefix JS
// RUN: %hermes -O -emit-binary -output-source-map -out %t.hbc %s && %hermes %t.hbc \
// RUN:   | %FileCheck --match-full-lines %s -check-prefix BC

"use strict";

function fn1() {}
print(loc(fn1).fileName);
// JS: {{.+}}/hermes-internal-get-function-location.js
// BC: {{.+}}.hbc
print(loc(fn1).lineNumber);
// JS: [[@LINE-5]]
// BC: undefined
print(loc(fn1).columnNumber);
// JS-NEXT: 1
// BC-NEXT: undefined
print(loc(fn1).segmentID);
// JS-NEXT: undefined
// BC-NEXT: 0
print(loc(fn1).virtualOffset);
// JS-NEXT: undefined
// BC-NEXT: [[FN1OFFSET:[0-9]+]]
print(loc(fn1).isNative);
// JS-NEXT: false
// BC-NEXT: false

const fn1Bound = fn1.bind(null);
print(loc(fn1Bound).fileName);
// JS-NEXT: {{.+}}/hermes-internal-get-function-location.js
// BC-NEXT: {{.+}}.hbc
print(loc(fn1Bound).lineNumber);
// JS: [[@LINE-25]]
// BC-NEXT: undefined
print(loc(fn1Bound).columnNumber);
// JS-NEXT: 1
// BC-NEXT: undefined
print(loc(fn1Bound).segmentID);
// JS-NEXT: undefined
// BC-NEXT: 0
print(loc(fn1Bound).virtualOffset);
// JS-NEXT: undefined
// BC-NEXT: [[FN1OFFSET]]
print(loc(fn1Bound).isNative);
// JS-NEXT: false
// BC-NEXT: false

print(loc(Object).fileName);
// JS-NEXT: undefined
// BC-NEXT: undefined
print(loc(Object).lineNumber);
// JS-NEXT: undefined
// BC-NEXT: undefined
print(loc(Object).columnNumber);
// JS-NEXT: undefined
// BC-NEXT: undefined
print(loc(Object).segmentID);
// JS-NEXT: undefined
// BC-NEXT: undefined
print(loc(Object).virtualOffset);
// JS-NEXT: undefined
// BC-NEXT: undefined
print(loc(Object).isNative)
// JS-NEXT: true
// BC-NEXT: true

const fn2 = eval('(x => x()) \n//# sourceURL=dummy');
print(loc(fn2).fileName);
// JS-NEXT: dummy
// BC-NEXT: dummy
print(loc(fn2).lineNumber);
// JS-NEXT: 1
// BC-NEXT: 1
print(loc(fn2).columnNumber);
// JS-NEXT: 2
// BC-NEXT: 2
print(loc(fn2).segmentID);
// JS-NEXT: undefined
// BC-NEXT: undefined
print(loc(fn2).virtualOffset);
// JS-NEXT: undefined
// BC-NEXT: undefined
print(loc(fn2).isNative);
// JS-NEXT: false
// BC-NEXT: false

try { loc(); } catch(e) { print(e.message); }
// JS-NEXT: Argument to HermesInternal.getFunctionLocation must be callable
// BC-NEXT: Argument to HermesInternal.getFunctionLocation must be callable
try { loc({}); } catch(e) { print(e.message); }
// JS-NEXT: Argument to HermesInternal.getFunctionLocation must be callable
// BC-NEXT: Argument to HermesInternal.getFunctionLocation must be callable
try { loc(Math.PI); } catch(e) { print(e.message); }
// JS-NEXT: Argument to HermesInternal.getFunctionLocation must be callable
// BC-NEXT: Argument to HermesInternal.getFunctionLocation must be callable

function loc(f) { return HermesInternal.getFunctionLocation(f); }
