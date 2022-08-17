/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 * @flow
 */

const dependencyExtractor = require('./emdependency-extractor.js')();

const hermesExtractDependencies = dependencyExtractor.cwrap(
  'hermesExtractDependencies',
  'number',
  ['number', 'number', 'string']
);

const hermesDependencies_free = dependencyExtractor.cwrap(
  'hermesDependencies_free',
  'void',
  ['number']
);
const hermesDependencies_getError = dependencyExtractor.cwrap(
  'hermesDependencies_getError',
  'string',
  ['number']
);
const hermesDependencies_getDeps = dependencyExtractor.cwrap(
  'hermesDependencies_getDeps',
  'string',
  ['number']
);

/*
type Dependency = {|
  name: string,
  kind: string,
|}
*/

function extractDependencies(
  source /*: string | Buffer */
) /*: Dependency[] */ {
  // If the input is a string, convert it to a Buffer.
  // Otherwise assume it is a Buffer.
  const buffer =
    typeof source === 'string' ? Buffer.from(source, 'utf8') : source;

  const addr = dependencyExtractor._malloc(buffer.length + 1);
  if (!addr) {
    throw new Error('extractor out of memory');
  }

  try {
    // Copy the string into the WASM heap.
    dependencyExtractor.HEAP8.set(buffer, addr);
    // Zero terminate.
    dependencyExtractor.HEAP8[addr + buffer.length] = 0;

    const res = hermesExtractDependencies(addr, buffer.length + 1);
    try {
      const err = hermesDependencies_getError(res);
      if (err) {
        throw new Error(err);
      }
      const depsString = hermesDependencies_getDeps(res);
      const deps = JSON.parse(depsString);
      return deps;
    } finally {
      hermesDependencies_free(res);
    }
  } finally {
    dependencyExtractor._free(addr);
  }
}

exports.extractDependencies = extractDependencies;
