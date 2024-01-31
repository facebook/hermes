/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 *
 * @format
 */

const path = require('path');

const FLOW_BUNDLER_ROOT = path.resolve(
  __dirname,
  '../../build-helpers/flow-bundler',
);

function createConfig(benchmarkName) {
  return {
    root: path.join(__dirname, 'src'),
    outDir: path.join(__dirname, 'out'),
    entrypoints: [`./app/${benchmarkName}/index.js`],
    simpleJsxTransform: true,
    out: {
      [`${benchmarkName}.js`]: null,
      [`${benchmarkName}-stripped.js`]: {
        babelConfig: {
          plugins: [
            // This plugin lives in the flow-bundler repo
            require.resolve('@babel/plugin-transform-flow-strip-types', {
              paths: [FLOW_BUNDLER_ROOT],
            }),
          ],
        },
      },
      [`${benchmarkName}-es5.js`]: {
        babelConfig: {
          presets: [
            [
              // This preset lives in the flow-bundler repo
              require.resolve('metro-react-native-babel-preset', {
                paths: [FLOW_BUNDLER_ROOT],
              }),
              {enableBabelRuntime: false},
            ],
          ],
        },
      },
    },
  };
}

module.exports = {
  builds: [createConfig('simple'), createConfig('music')],
};
