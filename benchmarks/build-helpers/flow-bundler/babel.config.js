/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * @format
 */

const path = require('path');
const NODE_MODULES = path.resolve(__dirname, 'node_modules');

module.exports = {
  presets: [
    [
      path.join(NODE_MODULES, '@babel/preset-env'),
      {targets: {node: 'current'}},
    ],
    path.join(NODE_MODULES, '@babel/preset-flow'),
  ],
  plugins: [path.join(NODE_MODULES, 'babel-plugin-syntax-hermes-parser')],
  ignore: [/\/node_modules\//],
};
