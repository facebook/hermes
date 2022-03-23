/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

// Safe to access `stack` within the callback.
Error.prepareStackTrace = (e, callSites) => e.stack;
print((new Error).stack);
//CHECK: Error
//CHECK-NEXT: at global ({{.*/error-prepare-stack-trace\.js}}:13:8)

Error.prepareStackTrace = (e, callSites) => callSites;

printCallSites(new Error().stack);
//CHECK-NEXT: global @ {{.*/error-prepare-stack-trace\.js}}:19:25
//CHECK-NEXT:   getEvalOrigin(): null
//CHECK-NEXT:   getFunction(): undefined
//CHECK-NEXT:   getMethodName(): null
//CHECK-NEXT:   getPromiseIndex(): null
//CHECK-NEXT:   getThis(): undefined
//CHECK-NEXT:   getTypeName(): null
//CHECK-NEXT:   isAsync(): false
//CHECK-NEXT:   isConstructor(): null
//CHECK-NEXT:   isEval(): null
//CHECK-NEXT:   isNative(): false
//CHECK-NEXT:   isPromiseAll(): false
//CHECK-NEXT:   isToplevel(): null
//CHECK-NEXT:   getBytecodeAddress(): {{[0-9]+}}

printCallSites([1].map(_ => Error().stack)[0]);
//CHECK-NEXT: null @ {{.*/error-prepare-stack-trace\.js}}:35:34
//CHECK-NEXT:   getEvalOrigin(): null
//CHECK-NEXT:   getFunction(): undefined
//CHECK-NEXT:   getMethodName(): null
//CHECK-NEXT:   getPromiseIndex(): null
//CHECK-NEXT:   getThis(): undefined
//CHECK-NEXT:   getTypeName(): null
//CHECK-NEXT:   isAsync(): false
//CHECK-NEXT:   isConstructor(): null
//CHECK-NEXT:   isEval(): null
//CHECK-NEXT:   isNative(): false
//CHECK-NEXT:   isPromiseAll(): false
//CHECK-NEXT:   isToplevel(): null
//CHECK-NEXT:   getBytecodeAddress(): {{[0-9]+}}
//CHECK-NEXT: map @ null:null:null
//CHECK-NEXT:   getEvalOrigin(): null
//CHECK-NEXT:   getFunction(): undefined
//CHECK-NEXT:   getMethodName(): null
//CHECK-NEXT:   getPromiseIndex(): null
//CHECK-NEXT:   getThis(): undefined
//CHECK-NEXT:   getTypeName(): null
//CHECK-NEXT:   isAsync(): false
//CHECK-NEXT:   isConstructor(): null
//CHECK-NEXT:   isEval(): null
//CHECK-NEXT:   isNative(): true
//CHECK-NEXT:   isPromiseAll(): false
//CHECK-NEXT:   isToplevel(): null
//CHECK-NEXT:   getBytecodeAddress(): null
//CHECK-NEXT: global @ {{.*/error-prepare-stack-trace\.js}}:35:23
//CHECK-NEXT:   getEvalOrigin(): null
//CHECK-NEXT:   getFunction(): undefined
//CHECK-NEXT:   getMethodName(): null
//CHECK-NEXT:   getPromiseIndex(): null
//CHECK-NEXT:   getThis(): undefined
//CHECK-NEXT:   getTypeName(): null
//CHECK-NEXT:   isAsync(): false
//CHECK-NEXT:   isConstructor(): null
//CHECK-NEXT:   isEval(): null
//CHECK-NEXT:   isNative(): false
//CHECK-NEXT:   isPromiseAll(): false
//CHECK-NEXT:   isToplevel(): null
//CHECK-NEXT:   getBytecodeAddress(): {{[0-9]+}}

// Non-callable values of prepareStackTrace are silently ignored.
Error.prepareStackTrace = {};
print((new Error).stack);
//CHECK: Error
//CHECK-NEXT: at global ({{.*/error-prepare-stack-trace\.js}}:81:8)

function printCallSite(callSite) {
  var s = '';
  s += callSite.getFunctionName();
  s += ' @ ';
  s += callSite.getFileName();
  s += ':';
  s += callSite.getLineNumber();
  s += ':';
  s += callSite.getColumnNumber();
  s += '\n';
  s += '  getEvalOrigin(): ' + callSite.getEvalOrigin() + '\n';
  s += '  getFunction(): ' + callSite.getFunction() + '\n';
  s += '  getMethodName(): ' + callSite.getMethodName() + '\n';
  s += '  getPromiseIndex(): ' + callSite.getPromiseIndex() + '\n';
  s += '  getThis(): ' + callSite.getThis() + '\n';
  s += '  getTypeName(): ' + callSite.getTypeName() + '\n';
  s += '  isAsync(): ' + callSite.isAsync() + '\n';
  s += '  isConstructor(): ' + callSite.isConstructor() + '\n';
  s += '  isEval(): ' + callSite.isEval() + '\n';
  s += '  isNative(): ' + callSite.isNative() + '\n';
  s += '  isPromiseAll(): ' + callSite.isPromiseAll() + '\n';
  s += '  isToplevel(): ' + callSite.isToplevel() + '\n';
  s += '  getBytecodeAddress(): ' + callSite.getBytecodeAddress() + '\n';
  print(s.trim());
}

function printCallSites(callSites) {
  callSites.forEach(printCallSite);
}
