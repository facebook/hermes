/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const HermesParserWASM = require('./HermesParserWASM');

const hermesParse = HermesParserWASM.cwrap('hermesParse', 'number', [
  'number',
  'number',
  'string',
]);

const hermesParseResult_free = HermesParserWASM.cwrap(
  'hermesParseResult_free',
  'void',
  ['number'],
);

const hermesParseResult_getError = HermesParserWASM.cwrap(
  'hermesParseResult_getError',
  'string',
  ['number'],
);

const hermesParseResult_getASTReference = HermesParserWASM.cwrap(
  'hermesParseResult_getASTReference',
  'number',
  ['number'],
);

function parse(code) {
  const buffer = Buffer.from(code, 'utf8');
  const addr = HermesParserWASM._malloc(buffer.length + 1);
  if (!addr) {
    throw new Error('Parser out of memory');
  }

  try {
    // Copy the string into the WASM heap and null-terminate
    HermesParserWASM.HEAP8.set(buffer, addr);
    HermesParserWASM.HEAP8[addr + buffer.length] = 0;

    const parseResult = hermesParse(addr, buffer.length + 1);
    try {
      // Extract and throw error from parse result if parsing failed
      const err = hermesParseResult_getError(parseResult);
      if (err) {
        throw new Error(err);
      }

      // Find root AST mode from reference
      const astReference = hermesParseResult_getASTReference(parseResult);
      return HermesParserWASM.JSReferences.pop(astReference);
    } finally {
      hermesParseResult_free(parseResult);
    }
  } finally {
    HermesParserWASM._free(addr);
  }
}

module.exports = {
  parse,
};
