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

/*::
import type {BuildType} from './version-utils';
*/

const {getCurrentCommit, isGitRepo} = require('./scm-utils');

function main() {
  console.log(`Is git repo: ${isGitRepo()}`);
  console.log(`Current commit: ${getCurrentCommit()}`);
}

if (require.main === module) {
  void main();
}
