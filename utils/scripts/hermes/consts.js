/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

const path = require('path');

/**
 * The absolute path to the repo root.
 */
const REPO_ROOT /*: string */ = path.resolve(__dirname, '../../..');

/**
 * The absolute path to the android directory.
 */
const ANDROID_DIR /*: string */ = path.join(REPO_ROOT, 'android');

module.exports = {
  REPO_ROOT,
  ANDROID_DIR,
};
