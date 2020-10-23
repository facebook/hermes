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

  return options;
}

function getAdapter(options) {
  return options.babel === true
    ? new HermesToBabelAdapter(options)
    : new HermesToESTreeAdapter(options);
}

function parse(code, opts = {}) {
  const options = getOptions(opts);
  const ast = HermesParser.parse(code, options.sourceFilename);
  const adapter = getAdapter(options);

  return adapter.transform(ast);
}

module.exports = {parse};
