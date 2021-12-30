/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const CaseSensitivePathsPlugin = require('case-sensitive-paths-webpack-plugin');

module.exports = function(context, options) {
  return {
    name: 'case-sensitive-paths',
    configureWebpack(config, isServer) {
      return {
        plugins: [
          new CaseSensitivePathsPlugin()
        ],
      };
    },
  };
};
