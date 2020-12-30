/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const HermesParser = require('./HermesParser');
const HermesToBabelAdapter = require('./HermesToBabelAdapter');
const HermesToESTreeAdapter = require('./HermesToESTreeAdapter');

function getOptions(options) {
  // Default filename to null if none was provided
  if (options.sourceFilename == null) {
    options.sourceFilename = null;
  }

  // Default to detecting whether to parse Flow syntax by the presence
  // of an @flow pragma.
  if (options.flow == null) {
    options.flow = 'detect';
  } else if (options.flow != 'all' && options.flow != 'detect') {
    throw new Error('flow option must be "all" or "detect"');
  }

  options.tokens = options.tokens === true;

  return options;
}

function getAdapter(options, code) {
  return options.babel === true
    ? new HermesToBabelAdapter(options)
    : new HermesToESTreeAdapter(options, code);
}

function parse(code, opts = {}) {
  const options = getOptions(opts);
  const ast = HermesParser.parse(code, options);
  const adapter = getAdapter(options, code);

  return adapter.transform(ast);
}

module.exports = {parse};
