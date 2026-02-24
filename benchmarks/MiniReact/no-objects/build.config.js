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
      [`${benchmarkName}-lowered.js`]: {
        babelConfig: {
          plugins: [
            [
              require.resolve('@babel/plugin-transform-class-properties', {
                paths: [FLOW_BUNDLER_ROOT],
              }),
              {enableBabelRuntime: false},
            ],
            // This plugin lives in the flow-bundler repo
            require.resolve('@babel/plugin-transform-flow-strip-types', {
              paths: [FLOW_BUNDLER_ROOT],
            }),
            [
              require.resolve('@babel/plugin-transform-classes', {
                paths: [FLOW_BUNDLER_ROOT],
              }),
              {enableBabelRuntime: false},
            ],
          ],
          assumptions: {
            constantSuper: true,
            noClassCalls: true,
            setClassMethods: true,
            setPublicClassFields: true,
            superIsCallableConstructor: true,
          },
        },
      },
    },
  };
}

module.exports = {
  builds: [createConfig('simple'), createConfig('music')],
};
