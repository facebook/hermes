/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// @flow
// @format

const hermesc = require("./emhermesc.js")({
    noInitialRun: true,
    noExitRuntime: true,
});

const hermesCompileToBytecode = hermesc.cwrap("hermesCompileToBytecode", "number",
    ["number", "number", "string"]);
const hermesCompileResult_getError = hermesc.cwrap("hermesCompileResult_getError", "string",
    ["number"]);
const hermesCompileResult_getBytecodeAddr = hermesc.cwrap("hermesCompileResult_getBytecodeAddr", "number",
    ["number"]);
const hermesCompileResult_getBytecodeSize = hermesc.cwrap("hermesCompileResult_getBytecodeSize", "number",
    ["number"]);
const hermesCompileResult_free = hermesc.cwrap("hermesCompileResult_free", "void",
    ["number"]);

function compile(source /*: string | Buffer*/, sourceURL /*: string*/) /*: Buffer*/ {
    // If the input is a string, convert it to a Buffer, otherwise assume it is a Buffer.
    const buffer =
        typeof source === 'string' ? Buffer.from(source, 'utf8') : source;

    const addr = hermesc._malloc(buffer.length + 1);
    if (!addr) {
        throw new Error('Hermesc out of memory');
    }

    try {
        // Copy the string into the WASM heap.
        hermesc.HEAP8.set(buffer, addr);
        // Zero terminate.
        hermesc.HEAP8[addr + buffer.length] = 0;

        const res = hermesCompileToBytecode(addr, buffer.length + 1, sourceURL || "");
        try {
            const err = hermesCompileResult_getError(res);
            if (err) {
                throw new Error(err);
            }
            const bcAddr = hermesCompileResult_getBytecodeAddr(res);
            const bcSize = hermesCompileResult_getBytecodeSize(res);
            // This creates a view of the specified section of the buffer.
            const resView = Buffer.from(hermesc.HEAP8.buffer, bcAddr, bcSize);
            // This is the actual copy that we want to return.
            return Buffer.from(resView);
        } finally {
            hermesCompileResult_free(res);
        }
    } finally {
        hermesc._free(addr);
    }
}

exports.compile = compile;
