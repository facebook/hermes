/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');

module.exports = function(context, options) {
  return {
    name: 'monaco-editor',
    configureWebpack(config, isServer) {
      return {
        module: {
          rules: [
            {
              test: /\.ttf$/,
              use: ['file-loader'],
            },
          ],
        },
        plugins: [
          new MonacoWebpackPlugin({
            languages: ['javascript', 'json'],
          }),
        ],
      };
    },
  };
};
