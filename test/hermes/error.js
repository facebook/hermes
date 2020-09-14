/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var e = new Error();
print(e);
//CHECK: Error

print(e.hasOwnProperty(e, 'name'));
//CHECK: false
print(e.hasOwnProperty(e, 'message'));
//CHECK: false
print(e.__proto__.hasOwnProperty('name'));
//CHECK: true
print(e.__proto__.hasOwnProperty('message'));
//CHECK: true

try { Error.prototype.toString.call(undefined); } catch (e) { print(e.name); }
//CHECK: TypeError
try { Error.prototype.toString.call('string'); } catch (e) { print(e.name); }
//CHECK: TypeError
try { Error.prototype.toString.call(11); } catch (e) { print(e.name); }
//CHECK: TypeError
try { Error.prototype.toString.call(Symbol()); } catch (e) { print(e.name); }
//CHECK: TypeError

e.name = 'RandomError';
print(e);
//CHECK: RandomError

e.message = 'random-message';
print(e);
//CHECK: RandomError: random-message

e.name = '';
print(e);
//CHECK: random-message

e = new Error(undefined);
print(e.hasOwnProperty(e, 'message'));
//CHECK: false

e = new Error('another-message');
print(e);
//CHECK: Error: another-message

e = new TypeError();
print(e);
//CHECK: TypeError

print(e.hasOwnProperty(e, 'name'));
//CHECK: false
print(e.hasOwnProperty(e, 'message'));
//CHECK: false
print(e.__proto__.hasOwnProperty('name'));
//CHECK: true
print(e.__proto__.hasOwnProperty('message'));
//CHECK: true

e = new ReferenceError('ref');
print(e);
//CHECK: ReferenceError: ref

print(ReferenceError.__proto__ === Error);
//CHECK: true
print(EvalError.__proto__ === Error);
//CHECK: true

e = new Error();
e.name = 12345;
print(e);
// CHECK: 12345
e.message = 67890;
print(e);
// CHECK: 12345: 67890

// Check exception case of accessing Error.stack from a different 'this'
try {
  new Error().__lookupGetter__("stack").call({});
} catch (e) {
  print(e.name);
}
//CHECK: TypeError
