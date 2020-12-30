/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const HermesParser = require('../hermes-parser');
const VisitorKeys = require('./HermesESLintVisitorKeys');

function parseForESLint(code) {
  const ast = HermesParser.parse(code, {tokens: true});

  return {
    ast,
    visitorKeys: VisitorKeys,
  };
}

module.exports = {parseForESLint};
