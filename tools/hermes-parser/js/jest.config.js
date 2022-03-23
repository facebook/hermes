/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

'use strict';

const path = require('path');
const packageJson = require('./package.json');

module.exports = {
  // All imported modules in your tests should be mocked automatically
  automock: false,
  // Automatically clear mock calls and instances between every test
  clearMocks: true,
  // Indicates whether the coverage information should be collected while executing the test
  collectCoverage: false,
  // The glob patterns Jest uses to detect test files
  testMatch: ['**/__tests__/**/*-test.js'],
  // An array of regexp pattern strings that are matched against all test paths, matched tests are skipped
  testPathIgnorePatterns: ['/node_modules/'],
  // An array of regexp pattern strings that are matched against all source file paths, matched files will skip transformation
  transformIgnorePatterns: ['/node_modules/', '/dist/'],

  // These mappings tell jest how to find the source files directly instead of using the built `dist` files.
  // This allows us to run tests without first doing a build.
  //
  // We have to manually teach jest about the generated files that get placed in the `dist` folders.
  // Otherwise jest will look in the `src` folder and will only find the unusable `.flow` file.
  moduleNameMapper: ({
    // the modules themselves
    ...Object.fromEntries(
      packageJson.workspaces.map(moduleName => [
        `^${moduleName}$`,
        path.resolve(__dirname, moduleName, 'src', 'index.js'),
      ]),
    ),

    // hermes-eslint
    '.*/HermesESLintVisitorKeys$': path.resolve(
      __dirname,
      'hermes-eslint',
      'dist',
      'HermesESLintVisitorKeys.js',
    ),

    // hermes-parser
    '.*/HermesParserNodeDeserializers$': path.resolve(
      __dirname,
      'hermes-parser',
      'dist',
      'HermesParserNodeDeserializers.js',
    ),
    '.*/HermesParserWASM$': path.resolve(
      __dirname,
      'hermes-parser',
      'dist',
      'HermesParserWASM.js',
    ),
    '.*/HermesAST$': path.resolve(
      __dirname,
      'hermes-parser',
      'dist',
      'HermesAST.js',
    ),
    '.*/generated/visitor-keys$': path.resolve(
      __dirname,
      'hermes-parser',
      'dist',
      'generated',
      'visitor-keys.js',
    ),

    // hermes-transform
    '.*/generated/node-types$': path.resolve(
      __dirname,
      'hermes-transform',
      'dist',
      'generated',
      'node-types.js',
    ),
    '.*/generated/special-case-node-types$': path.resolve(
      __dirname,
      'hermes-transform',
      'dist',
      'generated',
      'special-case-node-types.js',
    ),
    '.*/generated/TransformCloneSignatures$': path.resolve(
      __dirname,
      'hermes-transform',
      'dist',
      'generated',
      'TransformCloneSignatures.js',
    ),
    '.*/generated/TransformReplaceSignatures$': path.resolve(
      __dirname,
      'hermes-transform',
      'dist',
      'generated',
      'TransformReplaceSignatures.js',
    ),
  } /*: {[string]: string} */),
};
