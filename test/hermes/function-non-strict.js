/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var nonStrict = function() { return 1; };

var strict = function() {
  "use strict";
  return 1;
};

print('function properties');
// CHECK-LABEL: function properties
print(typeof strict, typeof nonStrict);
// CHECK-NEXT: function function
try { print(nonStrict.caller); } catch(e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError Restricted property cannot be accessed
try { print(nonStrict.arguments); } catch(e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError Restricted property cannot be accessed
try { print(strict.caller); } catch(e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError Restricted property cannot be accessed
try { print(strict.arguments); } catch(e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError Restricted property cannot be accessed

var bound = nonStrict.bind(42);
try { print(bound.caller); } catch(e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError Restricted property cannot be accessed
try { print(bound.arguments); } catch(e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError Restricted property cannot be accessed

try { print(Function.prototype.caller); } catch(e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError Restricted property cannot be accessed
try { print(Function.prototype.arguments); } catch(e) { print('caught', e.name, e.message); }
// CHECK-NEXT: caught TypeError Restricted property cannot be accessed
