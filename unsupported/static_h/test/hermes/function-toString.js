/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// UNSUPPORTED: serializer

print('Function.prototype.toString');
// CHECK-LABEL: Function.prototype.toString

/// --- Basics ---

function dflt(x) {};
print(dflt.toString());
// CHECK-NEXT: function dflt(a0) { [bytecode] }

function showSource(x) { 'show source' }
print(showSource.toString());
// CHECK-NEXT: function showSource(x) { 'show source' } 

function hideSource(x) { 'hide source' }
print(hideSource.toString());
// CHECK-NEXT: function hideSource() { [native code] } 

function sensitive(x) { 'sensitive'; }
print(sensitive.toString());
// CHECK-NEXT: function sensitive() { [native code] } 


/// --- Implementation-hidden Functions ---
/// Are indistinguishable with real 0-arity NativeFunctions.

print(Date.now.toString());
// CHECK-NEXT: function now() { [native code] }

print((function now() { 'hide source' }).toString());
// CHECK-NEXT: function now() { [native code] }


/// --- Function Expression ---

print((function fe() { 'show source' }).toString());
// CHECK-NEXT: function fe() { 'show source' }


/// --- Arrow Function Expression ---

print((() => { 'show source' }).toString());
// CHECK-NEXT: () => { 'show source' }

print((() => { 'hide source' }).toString());
// CHECK-NEXT: function () { [native code] }


/// --- Async / Generator Function ---
/// You can tell if a function is async/generator for 
/// both the Default and ShowSource source visibility, but
/// not the HideSource and Sensitive.

async function af1() { await 1; };
async function af2() { 'show source'; await 1; };
async function af3() { 'hide source'; await 1; };
async function af4() { 'sensitive'; await 1; };
print(af1.toString());
print(af2.toString());
print(af3.toString());
print(af4.toString());
// CHECK-NEXT: async function af1() { [bytecode] }
// CHECK-NEXT: async function af2() { 'show source'; await 1; }
// CHECK-NEXT: function af3() { [native code] }
// CHECK-NEXT: function af4() { [native code] }

function* gf1() { yield 1; };
function* gf2() { 'show source'; yield 1; };
function* gf3() { 'hide source'; yield 1; };
function* gf4() { 'sensitive'; yield 1; };
print(gf1.toString());
print(gf2.toString());
print(gf3.toString());
print(gf4.toString());
// CHECK-NEXT: function *gf1() { [bytecode] }
// CHECK-NEXT: function* gf2() { 'show source'; yield 1; }
// CHECK-NEXT: function gf3() { [native code] }
// CHECK-NEXT: function gf4() { [native code] }


/// --- "Large" Source ---

function large() {
  'show source';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
  '0123456789abcdef';
}
print(large.toString());
// CHECK-LABEL:function large() {
// CHECK-NEXT:  'show source';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:  '0123456789abcdef';
// CHECK-NEXT:}
