/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// @format

const hc = require("./HermesCompiler.js");
const assert = require("assert");

function compileOrError(str) {
    try {
        let buffer = hc.compile(str);
        console.log("bytecode buffer length", buffer.length)
    } catch (e) {
        console.error("Hermes error:", e.message);
        return false;
    }
    return true;
}

assert(compileOrError("var x = 1; print(x);"));
assert(!compileOrError("var x = 1 + ; print(x);"));
