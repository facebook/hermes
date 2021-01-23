/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const HermesParser = require('hermes-parser');
const VisitorKeys = require('./HermesESLintVisitorKeys');

function parse(code) {
  return HermesParser.parse(code, {tokens: true});
}

function parseForESLint(code) {
  const ast = parse(code);

  return {
    ast,
    visitorKeys: VisitorKeys,
  };
}

module.exports = {parse, parseForESLint};
