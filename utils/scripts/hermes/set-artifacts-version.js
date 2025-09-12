/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

/*::
import type {BuildType, Version} from './version-utils';
*/

const {ANDROID_DIR} = require('./consts');
const {
  getVersion,
  validateBuildType,
  updateGradlePropertiesFile,
} = require('./version-utils');
const {promises: fs} = require('fs');
const path = require('path');
const {parseArgs} = require('util');

const config = {
  options: {
    'build-type': {
      type: 'string',
      short: 'b',
    },
    help: {type: 'boolean'},
  },
};

async function main() {
  const {
    values: {'build-type': buildType, help},
    /* $FlowFixMe[incompatible-call] Natural Inference rollout. See
     * https://fburl.com/workplace/6291gfvu */
  } = parseArgs(config);

  if (help) {
    console.log(`
  Usage: node ./utils/scripts/hermes/set-artifacts-version.js [OPTIONS]

  Updates version in gradle.proparties file.

  Options:
    --build-type       One of ['dry-run', 'release'].
    `);
    return;
  }

  if (!validateBuildType(buildType)) {
    throw new Error(`Unsupported build type: ${buildType}`);
  }

  const version = await getVersion(buildType);
  await updateGradlePropertiesFile(version);
}

if (require.main === module) {
  void main();
}
