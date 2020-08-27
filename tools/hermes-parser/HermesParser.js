/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const hermesParserWASM = require('./hermes-parser-wasm.js');

const hermesParse = hermesParserWASM.cwrap('hermesParse', 'number', [
  'number',
  'number',
  'string',
]);

const hermesParseResult_free = hermesParserWASM.cwrap(
  'hermesParseResult_free',
  'void',
  ['number'],
);

const hermesParseResult_getError = hermesParserWASM.cwrap(
  'hermesParseResult_getError',
  'string',
  ['number'],
);

const hermesParseResult_getASTReference = hermesParserWASM.cwrap(
  'hermesParseResult_getASTReference',
  'number',
  ['number'],
);

function parse(code) {
  const buffer = Buffer.from(code, 'utf8');
  const addr = hermesParserWASM._malloc(buffer.length + 1);
  if (!addr) {
    throw new Error('Parser out of memory');
  }

  try {
    // Copy the string into the WASM heap and null-terminate
    hermesParserWASM.HEAP8.set(buffer, addr);
    hermesParserWASM.HEAP8[addr + buffer.length] = 0;

    const parseResult = hermesParse(addr, buffer.length + 1);
    try {
      // Extract and throw error from parse result if parsing failed
      const err = hermesParseResult_getError(parseResult);
      if (err) {
        const astReference = hermesParseResult_getASTReference(parseResult);
        hermesParserWASM.JSReferences.pop(astReference);
        throw new Error(err);
      }

      // Find root AST mode from reference
      const astReference = hermesParseResult_getASTReference(parseResult);
      return hermesParserWASM.JSReferences.pop(astReference);
    } finally {
      hermesParseResult_free(parseResult);
    }
  } finally {
    hermesParserWASM._free(addr);
  }
}

module.exports = {
  parse,
};
