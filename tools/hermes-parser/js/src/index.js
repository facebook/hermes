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

function getAdapter(options) {
  return options.babel === true
    ? new HermesToBabelAdapter()
    : new HermesToESTreeAdapter();
}

function parse(code, options = {}) {
  const ast = HermesParser.parse(code);
  const adapter = getAdapter(options);

  return adapter.transform(ast);
}

module.exports = {parse};
