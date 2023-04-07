/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s

var f;
var funcp;
var argumentsp;
var callee = _ => "callee";

function calleeDirectly(a) {
    return a.callee;
}

function calleeIndirectly(a) {
    return a[callee()];
}

print("directArgumentsCallee");
(function directArgumentsCallee(param) {
    try {
        funcp = arguments.callee;
    } catch (e) {
    }
    let f = () => {
        return param;
    };
    return f;
})();
f = funcp({ a: 1.1 });
f();

print("indirectArgumentsCallee");
(function indirectArgumentsCallee(param) {
    try {
        funcp = arguments.callee;
    } catch (e) {
    }
    let f = () => {
        return param;
    };
    return f;
})();
f = funcp({ a: 1.1 });
f();

print("passignArgumentsToAnotherFunction_directArgumentsCallee");
(function passignArgumentsToAnotherFunction_DirectArgumentsCallee(param) {
    try {
        funcp = calleeDirectly(arguments);
    } catch (e) {
    }
    let f = () => {
        return param;
    };
    return f;
})();
f = funcp({ a: 1.1 });
f();

print("passignArgumentsToAnotherFunction_indirectArgumentsCallee");
(function passignArgumentsToAnotherFunction_indirectArgumentsCallee(param) {
    try {
        funcp = calleeIndirectly(arguments);
    } catch (e) {
    }
    let f = () => {
        return param;
    };
    return f;
})();
f = funcp({ a: 1.1 });
f();

print("arrow_directArgumentsCallee");
(function arrow_directArgumentsCallee(param) {
    try {
        funcp = (_ => arguments.callee)();
    } catch (e) {
    }
    let f = () => {
        return param;
    };
    return f;
})();
f = funcp({ a: 1.1 });
f();

print("arrow_indirectArgumentsCallee");
(function arrow_indirectArgumentsCallee(param) {
    try {
        funcp = (_ => arguments[callee()])();
    } catch (e) {
    }
    let f = () => {
        return param;
    };
    return f;
})();
f = funcp({ a: 1.1 });
f();

print("capture_directArgumentsCallee");
(function capture_directArgumentsCallee(param) {
    try {
        argumentsp = arguments;
    } catch (e) {
    }
    let f = () => {
        return param;
    };
    return f;
})();
f = argumentsp.callee({ a: 1.1 });
f();

print("capture_indirectArgumentsCallee");
(function capture_indirectArgumentsCallee(param) {
    try {
        argumentsp = arguments;
    } catch (e) {
    }
    let f = () => {
        return param;
    };
    return f;
})();
f = argumentsp[callee()]({ a: 1.1 });
f();
