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

const {echo, exec} = require('shelljs');

function getCurrentCommit() /*: string */ {
  return isGitRepo()
    ? exec('git rev-parse HEAD', {
        silent: true,
      }).stdout.trim()
    : 'TEMP';
}

function isGitRepo() /*: boolean */ {
  try {
    return (
      exec('git rev-parse --is-inside-work-tree', {
        silent: true,
      }).stdout.trim() === 'true'
    );
  } catch (error) {
    echo(
      `It wasn't possible to check if we are in a git repository. Details: ${error}`,
    );
  }
  return false;
}

module.exports = {
  getCurrentCommit,
  isGitRepo,
};
