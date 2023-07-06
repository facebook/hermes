/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s

// Used to allocate too many handles.
var proxy = new Proxy(Promise, Promise);
proxy.get = function () { return proxy };
Promise.__proto__ = proxy;
try { Promise[0] = 1; } catch(e) { print('caught', e.name); }
// CHECK: caught RangeError
