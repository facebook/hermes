/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
//CHECK-NEXT: false
print(e.hasOwnProperty(e, 'message'));
//CHECK-NEXT: false
print(e.__proto__.hasOwnProperty('name'));
//CHECK-NEXT: true
print(e.__proto__.hasOwnProperty('message'));
//CHECK-NEXT: true

try { Error.prototype.toString.call(undefined); } catch (e) { print(e); }
//CHECK-NEXT: TypeError: Error.prototype.toString called on incompatible receiver undefined
try { Error.prototype.toString.call('string'); } catch (e) { print(e); }
//CHECK-NEXT: TypeError: Error.prototype.toString called on incompatible receiver 'string'
try { Error.prototype.toString.call(11); } catch (e) { print(e); }
//CHECK-NEXT: TypeError: Error.prototype.toString called on incompatible receiver 11
try { Error.prototype.toString.call(Symbol()); } catch (e) { print(e); }
//CHECK-NEXT: TypeError: Error.prototype.toString called on incompatible receiver Symbol()

print(Error.prototype.toString())
// CHECK-NEXT: Error
print(Object.prototype.toString.call(Error.prototype))
// CHECK-NEXT: [object Object]
print(Object.prototype.toString.call(new Error()))
// CHECK-NEXT: [object Error]

e.name = 'RandomError';
print(e);
//CHECK-NEXT: RandomError

e.message = 'random-message';
print(e);
//CHECK-NEXT: RandomError: random-message

e.name = '';
print(e);
//CHECK-NEXT: random-message

e = new Error(undefined);
print(e.hasOwnProperty(e, 'message'));
//CHECK-NEXT: false

e = new Error('another-message');
print(e);
//CHECK-NEXT: Error: another-message

e = new TypeError();
print(e);
//CHECK-NEXT: TypeError

print(e.hasOwnProperty(e, 'name'));
//CHECK-NEXT: false
print(e.hasOwnProperty(e, 'message'));
//CHECK-NEXT: false
print(e.__proto__.hasOwnProperty('name'));
//CHECK-NEXT: true
print(e.__proto__.hasOwnProperty('message'));
//CHECK-NEXT: true

e = new ReferenceError('ref');
print(e);
//CHECK-NEXT: ReferenceError: ref

print(ReferenceError.__proto__ === Error);
//CHECK-NEXT: true
print(EvalError.__proto__ === Error);
//CHECK-NEXT: true

e = new Error();
e.name = 12345;
print(e);
// CHECK-NEXT: 12345
e.message = 67890;
print(e);
// CHECK-NEXT: 12345: 67890

// Check exception case of accessing Error.stack from a different 'this'
try {
  new Error().__lookupGetter__("stack").call({});
} catch (e) {
  print(e.name);
}
//CHECK-NEXT: TypeError


// Regression test: setting the stack while getting the message causes a null dereference.
e = new Error();
Object.defineProperty(e, "message", {
  get: function () {
    e.stack = {}
  },
});
print(e.stack);
//CHECK-NEXT: Error
//CHECK-NEXT:     at global{{.*}}
