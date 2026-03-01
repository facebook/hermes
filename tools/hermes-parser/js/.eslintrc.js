/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

module.exports = {
  root: true,
  env: {
    commonjs: true,
    es2021: true,
    node: true,
  },
  plugins: ['@eslint-community/eslint-comments', 'fb-flow', 'ft-flow'],
  extends: [
    'eslint:recommended',
    'plugin:ft-flow/recommended',
    'plugin:@eslint-community/eslint-comments/recommended',
    'prettier',
  ],
  parser: require.resolve('./hermes-eslint'),
  rules: {
    curly: ['error', 'all'],
    eqeqeq: ['error', 'always', {null: 'never'}],
    'no-unused-vars': [
      'error',
      {
        args: 'all',
        varsIgnorePattern: '^_',
        argsIgnorePattern: '^_',
        ignoreRestSiblings: true,
      },
    ],

    '@eslint-community/eslint-comments/no-unused-disable': 'error',

    'fb-flow/flow-enums-default-if-possible': 'error',
    'fb-flow/no-flow-enums-object-mapping': 'error',
    'fb-flow/use-exact-by-default-object-type': 'error',
    'fb-flow/use-indexed-access-type': 'error',

    'ft-flow/array-style-complex-type': ['error', 'verbose'],
    'ft-flow/array-style-simple-type': ['error', 'verbose'],

    'no-undef': 'off',
    'ft-flow/define-flow-type': 'off',

    // from eslint-config-prettier, but for ft-flow instead of flowtype
    'ft-flow/boolean-style': 'off',
    'ft-flow/delimiter-dangle': 'off',
    'ft-flow/generic-spacing': 'off',
    'ft-flow/object-type-curly-spacing': 'off',
    'ft-flow/object-type-delimiter': 'off',
    'ft-flow/quotes': 'off',
    'ft-flow/semi': 'off',
    'ft-flow/space-after-type-colon': 'off',
    'ft-flow/space-before-generic-bracket': 'off',
    'ft-flow/space-before-type-colon': 'off',
    'ft-flow/union-intersection-spacing': 'off',
  },
  overrides: [
    {
      files: ['**/__tests__/**'],
      env: {
        'jest/globals': true,
      },
      settings: {
        version: require('jest/package.json').version,
      },
      plugins: ['jest'],
    },
    {
      files: ['**/__tests__/**/*-test.js'],
      extends: ['plugin:jest/recommended'],
      rules: {
        // we have tests which use utils which don't inline expects
        'jest/expect-expect': 'off',
        'jest/valid-title': 'off',
        'jest/no-disabled-tests': 'off',
      },
    },
  ],
  ignorePatterns: [
    'flow-typed',
    'node_modules',
    'dist',
    'fixtures/',
    'internal-prettier-v3/',
  ],
};
