/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// @format

const hc = require("./HermesCompiler.js");
const assert = require("assert");

function compileOrError(str) {
    try {
        console.log("Hermes compile: ");
        let buffer = hc.compile(str);
        console.log("bytecode buffer: ", buffer);
        console.log("bytecode buffer length: ", buffer.length)
    } catch (e) {
        console.error("Hermes error: ", e);
        return false;
    }
    return true;
}

// Convert thrown error to return value.
function wrap(func) {
    try {
        return func();
    } catch (e) {
        return e.message;
    }
}

// Test compile and validateBytecodeModule.
const goodCode = "var x = 1; print(x);"
const badCode = "var x = 1 + ; print(x);"

assert(compileOrError(goodCode));
assert(!compileOrError(badCode));

let goodBuf = hc.compile(goodCode);

let badBuf = Buffer.alloc(100);
assert.equal(wrap(() => hc.validateBytecodeModule(badBuf, 3)), "bytecode is not aligned to 4");
assert.equal(wrap(() => hc.validateBytecodeModule(badBuf, 0)), "bytecode buffer is too small");

badBuf = Buffer.alloc(200);
assert.equal(wrap(() => hc.validateBytecodeModule(badBuf, 0)), "bytecode buffer missing magic value");

badBuf = goodBuf.slice(0, 140);
assert.equal(wrap(() => hc.validateBytecodeModule(badBuf, 0)), "bytecode buffer is too small");

assert.notEqual(wrap(() => hc.validateBytecodeModule(goodBuf, 0)), 0);
