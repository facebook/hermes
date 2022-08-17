/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import commonjs from '@rollup/plugin-commonjs';
import { nodeResolve } from '@rollup/plugin-node-resolve'

export default {
  input: './index.js',
  output: {
    name: 'initPromise',
    file: 'Promise.js',
    format: 'iife'
  },
  plugins: [nodeResolve(), commonjs()]
};
