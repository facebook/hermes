/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// @flow
// @format

const hermesc = require(process.env.EMHERMESC || "./emhermesc.js")({
    noInitialRun: true,
    noExitRuntime: true,
});

const hermesCompileToBytecode = hermesc.cwrap("hermesCompileToBytecode", "number",
    ["number", "number", "string", "number", "number"]);
const hermesCompileResult_getError = hermesc.cwrap("hermesCompileResult_getError", "string",
    ["number"]);
const hermesCompileResult_getBytecodeAddr = hermesc.cwrap("hermesCompileResult_getBytecodeAddr", "number",
    ["number"]);
const hermesCompileResult_getBytecodeSize = hermesc.cwrap("hermesCompileResult_getBytecodeSize", "number",
    ["number"]);
const hermesCompileResult_free = hermesc.cwrap("hermesCompileResult_free", "void",
    ["number"]);

const hermesProps = JSON.parse(hermesc.ccall("hermesGetProperties", "string", [], []));

function strdup(str /*: string*/) /*: number*/ {
  var jsCopy = Buffer.from(str, 'utf8');
  var size = jsCopy.length + 1;
  var addr = hermesc._malloc(size);
  if(!addr) {
    throw new Error('hermesc string allocation error');
  }
  hermesc.HEAP8.set(jsCopy, addr);
  hermesc.HEAP8[addr+jsCopy.length] = 0;
  return { ptr: addr, size: size };
}

function compile(source /*: string | Buffer*/, options = {}) /*: Buffer*/ {
    const sourceURL = options.sourceURL || "";
    const sourceMap = options.sourceMap || "";

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

        // Strings are passed on the stack by default. Explicitly pass the source map
        // on the heap to avoid problems with large ones.
        let map = strdup(sourceMap);
        let res;
        try {
          res = hermesCompileToBytecode(addr, buffer.length + 1, sourceURL, map.ptr, map.size);
        } finally {
          hermesc._free(map.ptr);
        }
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

/// Check whether there is a valid bytecode module at the specified offset of
/// the input buffer.
/// \return the size of the module
/// \throw an error if the module is not valid.
function validateBytecodeModule(bc /*: Buffer*/, offset /*: number*/) /*: number*/ {
    // Check alignment.
    if ((bc.byteOffset + offset) % hermesProps.BYTECODE_ALIGNMENT) {
        throw Error("bytecode is not aligned to " + hermesProps.BYTECODE_ALIGNMENT);
    }

    // Check size.
    if (bc.length - offset < hermesProps.HEADER_SIZE) {
        throw Error("bytecode buffer is too small");
    }

    // Check magic.
    if (bc.readUInt32LE(offset + 0) !== hermesProps.MAGIC[0] ||
        bc.readUInt32LE(offset + 4) !== hermesProps.MAGIC[1]) {
        throw Error("bytecode buffer missing magic value");
    }

    // Check version.
    let version = bc.readUInt32LE(offset + 8);
    if (version !== hermesProps.VERSION) {
        throw Error("bytecode version is " + version + " but " + hermesProps.VERSION + " is required");
    }

    // Check reported size.
    let fileLength = bc.readUInt32LE(offset + hermesProps.LENGTH_OFFSET);
    if (bc.length - offset < fileLength) {
        throw Error("bytecode buffer is too small");
    }

    return (fileLength + hermesProps.BYTECODE_ALIGNMENT - 1) / hermesProps.BYTECODE_ALIGNMENT;
}

exports.compile = compile;
exports.validateBytecodeModule = validateBytecodeModule;
