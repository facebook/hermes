/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -Xmicrotask-queue -O -gc-sanitize-handles=1 -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec -Wx,-Xmicrotask-queue -Wx,-gc-sanitize-handles=1 %s | %FileCheck --match-full-lines %s

'use strict';

print('FinalizationRegistry');
// CHECK-LABEL: FinalizationRegistry

// Test that FinalizationRegistry exists and has correct @@toStringTag
print(FinalizationRegistry.prototype[Symbol.toStringTag]);
// CHECK-NEXT: FinalizationRegistry

// Test constructor requires 'new'
try {
  FinalizationRegistry(() => {});
} catch (e) {
  print('caught', e.name);
}
// CHECK-NEXT: caught TypeError

// Test constructor requires callable
try {
  new FinalizationRegistry(42);
} catch (e) {
  print('caught', e.name);
}
// CHECK-NEXT: caught TypeError

// Test constructor with valid callback
var fr = new FinalizationRegistry(heldValue => {
  print('cleanup', heldValue);
});
print(fr instanceof FinalizationRegistry);
// CHECK-NEXT: true

// Test register requires valid target
try {
  fr.register(42, 'held');
} catch (e) {
  print('caught', e.name, 'for primitive target');
}
// CHECK-NEXT: caught TypeError for primitive target

// Test register: target and heldValue cannot be the same
try {
  var obj = {};
  fr.register(obj, obj);
} catch (e) {
  print('caught', e.name, 'for same target and heldValue');
}
// CHECK-NEXT: caught TypeError for same target and heldValue

// Test register: unregisterToken must be valid if provided
try {
  fr.register({}, 'held', 42);
} catch (e) {
  print('caught', e.name, 'for invalid unregisterToken');
}
// CHECK-NEXT: caught TypeError for invalid unregisterToken

// Test register with valid arguments (no unregisterToken)
var target1 = {name: 'target1'};
print(fr.register(target1, 'heldValue1'));
// CHECK-NEXT: undefined

// Test register with valid arguments (with unregisterToken)
var target2 = {name: 'target2'};
var token2 = {name: 'token2'};
print(fr.register(target2, 'heldValue2', token2));
// CHECK-NEXT: undefined

// Test unregister requires valid token
try {
  fr.unregister(42);
} catch (e) {
  print('caught', e.name, 'for invalid unregister token');
}
// CHECK-NEXT: caught TypeError for invalid unregister token

// Test unregister returns false when no matching token
var unknownToken = {};
print('unregister unknown:', fr.unregister(unknownToken));
// CHECK-NEXT: unregister unknown: false

// Test unregister returns true when token matches
print('unregister token2:', fr.unregister(token2));
// CHECK-NEXT: unregister token2: true

// Test unregister returns false after already unregistered
print('unregister token2 again:', fr.unregister(token2));
// CHECK-NEXT: unregister token2 again: false

// Test register with Symbol as target
var symTarget = Symbol('mySymbol');
var fr2 = new FinalizationRegistry(() => {});
print(fr2.register(symTarget, 'symbol held value'));
// CHECK-NEXT: undefined

// Test register with registered Symbol throws
try {
  fr2.register(Symbol.for('registered'), 'held');
} catch (e) {
  print('caught', e.name, 'for registered symbol');
}
// CHECK-NEXT: caught TypeError for registered symbol

// Test multiple registrations with same token
var fr3 = new FinalizationRegistry(() => {});
var multiToken = {};
fr3.register({}, 'held1', multiToken);
fr3.register({}, 'held2', multiToken);
fr3.register({}, 'held3', multiToken);
// Unregister should remove all three
print('unregister multi:', fr3.unregister(multiToken));
// CHECK-NEXT: unregister multi: true
// Second unregister should return false
print('unregister multi again:', fr3.unregister(multiToken));
// CHECK-NEXT: unregister multi again: false

// Test register without unregisterToken (cannot be unregistered)
var fr4 = new FinalizationRegistry(() => {});
var target4 = {};
fr4.register(target4, 'held4');
// Try to unregister with target (won't work, need the token)
print('unregister without token:', fr4.unregister(target4));
// CHECK-NEXT: unregister without token: false

print('Done');
// CHECK-NEXT: Done
target1 = {};
gc();
// The callback should be run for target1 when performCheckpoint() is triggered in the end of hermes run.
// CHECK-NEXT: cleanup heldValue1
