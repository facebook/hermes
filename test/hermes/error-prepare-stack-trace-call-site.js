/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

Error.prepareStackTrace = (e, callSites) => callSites;
var callSite = new Error().stack[0];
Error.prepareStackTrace = undefined;

var callSiteMethods = Object.getOwnPropertyNames(callSite.__proto__).sort();

var invalidReceivers = [true, "\"this is a string\"", 1, undefined, null];

function testCallSiteMethodWithReceiver(name, receiver) {
    var prefix = receiver + "." + name + "(): ";

    try {
        callSite[name].apply(receiver);
    } catch (e) {
        return prefix + e.message;
    }
    return  prefix + "success.";
}

function testCallSiteMethodsWithReceiver(receiver) {
    return callSiteMethods.map(methodName =>
        testCallSiteMethodWithReceiver(methodName, receiver));
}

var results = invalidReceivers.map(testCallSiteMethodsWithReceiver).flat();
for (const i in results) {
    print(results[i]);
}

// CHECK: true.getBytecodeAddress(): CallSite method called on an incompatible receiver
// CHECK: true.getColumnNumber(): CallSite method called on an incompatible receiver
// CHECK: true.getEvalOrigin(): CallSite method called on an incompatible receiver
// CHECK: true.getFileName(): CallSite method called on an incompatible receiver
// CHECK: true.getFunction(): CallSite method called on an incompatible receiver
// CHECK: true.getFunctionName(): CallSite method called on an incompatible receiver
// CHECK: true.getLineNumber(): CallSite method called on an incompatible receiver
// CHECK: true.getMethodName(): CallSite method called on an incompatible receiver
// CHECK: true.getPromiseIndex(): CallSite method called on an incompatible receiver
// CHECK: true.getThis(): CallSite method called on an incompatible receiver
// CHECK: true.getTypeName(): CallSite method called on an incompatible receiver
// CHECK: true.isAsync(): CallSite method called on an incompatible receiver
// CHECK: true.isConstructor(): CallSite method called on an incompatible receiver
// CHECK: true.isEval(): CallSite method called on an incompatible receiver
// CHECK: true.isNative(): CallSite method called on an incompatible receiver
// CHECK: true.isPromiseAll(): CallSite method called on an incompatible receiver
// CHECK: true.isToplevel(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getBytecodeAddress(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getColumnNumber(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getEvalOrigin(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getFileName(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getFunction(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getFunctionName(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getLineNumber(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getMethodName(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getPromiseIndex(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getThis(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".getTypeName(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".isAsync(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".isConstructor(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".isEval(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".isNative(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".isPromiseAll(): CallSite method called on an incompatible receiver
// CHECK: "this is a string".isToplevel(): CallSite method called on an incompatible receiver
// CHECK: 1.getBytecodeAddress(): CallSite method called on an incompatible receiver
// CHECK: 1.getColumnNumber(): CallSite method called on an incompatible receiver
// CHECK: 1.getEvalOrigin(): CallSite method called on an incompatible receiver
// CHECK: 1.getFileName(): CallSite method called on an incompatible receiver
// CHECK: 1.getFunction(): CallSite method called on an incompatible receiver
// CHECK: 1.getFunctionName(): CallSite method called on an incompatible receiver
// CHECK: 1.getLineNumber(): CallSite method called on an incompatible receiver
// CHECK: 1.getMethodName(): CallSite method called on an incompatible receiver
// CHECK: 1.getPromiseIndex(): CallSite method called on an incompatible receiver
// CHECK: 1.getThis(): CallSite method called on an incompatible receiver
// CHECK: 1.getTypeName(): CallSite method called on an incompatible receiver
// CHECK: 1.isAsync(): CallSite method called on an incompatible receiver
// CHECK: 1.isConstructor(): CallSite method called on an incompatible receiver
// CHECK: 1.isEval(): CallSite method called on an incompatible receiver
// CHECK: 1.isNative(): CallSite method called on an incompatible receiver
// CHECK: 1.isPromiseAll(): CallSite method called on an incompatible receiver
// CHECK: 1.isToplevel(): CallSite method called on an incompatible receiver
// CHECK: undefined.getBytecodeAddress(): CallSite method called on an incompatible receiver
// CHECK: undefined.getColumnNumber(): CallSite method called on an incompatible receiver
// CHECK: undefined.getEvalOrigin(): CallSite method called on an incompatible receiver
// CHECK: undefined.getFileName(): CallSite method called on an incompatible receiver
// CHECK: undefined.getFunction(): CallSite method called on an incompatible receiver
// CHECK: undefined.getFunctionName(): CallSite method called on an incompatible receiver
// CHECK: undefined.getLineNumber(): CallSite method called on an incompatible receiver
// CHECK: undefined.getMethodName(): CallSite method called on an incompatible receiver
// CHECK: undefined.getPromiseIndex(): CallSite method called on an incompatible receiver
// CHECK: undefined.getThis(): CallSite method called on an incompatible receiver
// CHECK: undefined.getTypeName(): CallSite method called on an incompatible receiver
// CHECK: undefined.isAsync(): CallSite method called on an incompatible receiver
// CHECK: undefined.isConstructor(): CallSite method called on an incompatible receiver
// CHECK: undefined.isEval(): CallSite method called on an incompatible receiver
// CHECK: undefined.isNative(): CallSite method called on an incompatible receiver
// CHECK: undefined.isPromiseAll(): CallSite method called on an incompatible receiver
// CHECK: undefined.isToplevel(): CallSite method called on an incompatible receiver
// CHECK: null.getBytecodeAddress(): CallSite method called on an incompatible receiver
// CHECK: null.getColumnNumber(): CallSite method called on an incompatible receiver
// CHECK: null.getEvalOrigin(): CallSite method called on an incompatible receiver
// CHECK: null.getFileName(): CallSite method called on an incompatible receiver
// CHECK: null.getFunction(): CallSite method called on an incompatible receiver
// CHECK: null.getFunctionName(): CallSite method called on an incompatible receiver
// CHECK: null.getLineNumber(): CallSite method called on an incompatible receiver
// CHECK: null.getMethodName(): CallSite method called on an incompatible receiver
// CHECK: null.getPromiseIndex(): CallSite method called on an incompatible receiver
// CHECK: null.getThis(): CallSite method called on an incompatible receiver
// CHECK: null.getTypeName(): CallSite method called on an incompatible receiver
// CHECK: null.isAsync(): CallSite method called on an incompatible receiver
// CHECK: null.isConstructor(): CallSite method called on an incompatible receiver
// CHECK: null.isEval(): CallSite method called on an incompatible receiver
// CHECK: null.isNative(): CallSite method called on an incompatible receiver
// CHECK: null.isPromiseAll(): CallSite method called on an incompatible receiver
// CHECK: null.isToplevel(): CallSite method called on an incompatible receiver
