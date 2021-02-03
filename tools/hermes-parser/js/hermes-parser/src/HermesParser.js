/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const HermesParserDeserializer = require('./HermesParserDeserializer');
const HermesParserWASM = require('./HermesParserWASM');

const hermesParse = HermesParserWASM.cwrap('hermesParse', 'number', [
  'number',
  'number',
  'number',
  'number',
  'number',
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

const hermesParseResult_getProgramBuffer = HermesParserWASM.cwrap(
  'hermesParseResult_getProgramBuffer',
  'number',
  ['number'],
);

const hermesParseResult_getPositionBuffer = HermesParserWASM.cwrap(
  'hermesParseResult_getPositionBuffer',
  'number',
  ['number'],
);

const hermesParseResult_getPositionBufferSize = HermesParserWASM.cwrap(
  'hermesParseResult_getPositionBufferSize',
  'number',
  ['number'],
);

// Copy a string into the WASM heap and null-terminate
function copyToHeap(buffer, addr) {
  HermesParserWASM.HEAP8.set(buffer, addr);
  HermesParserWASM.HEAP8[addr + buffer.length] = 0;
}

function parse(source, options) {
  // Allocate space on heap for source text
  const sourceBuffer = Buffer.from(source, 'utf8');
  const sourceAddr = HermesParserWASM._malloc(sourceBuffer.length + 1);
  if (!sourceAddr) {
    throw new Error('Parser out of memory');
  }

  try {
    // Copy source text onto WASM heap
    copyToHeap(sourceBuffer, sourceAddr);

    const parseResult = hermesParse(
      sourceAddr,
      sourceBuffer.length + 1,
      options.flow === 'detect',
      options.tokens,
      options.allowReturnOutsideFunction,
    );

    try {
      // Extract and throw error from parse result if parsing failed
      const err = hermesParseResult_getError(parseResult);
      if (err) {
        throw new SyntaxError(err);
      }

      const deserializer = new HermesParserDeserializer(
        hermesParseResult_getProgramBuffer(parseResult),
        hermesParseResult_getPositionBuffer(parseResult),
        hermesParseResult_getPositionBufferSize(parseResult),
        HermesParserWASM,
        options,
      );
      return deserializer.deserialize();
    } finally {
      hermesParseResult_free(parseResult);
    }
  } finally {
    HermesParserWASM._free(sourceAddr);
  }
}

module.exports = {
  parse,
};
