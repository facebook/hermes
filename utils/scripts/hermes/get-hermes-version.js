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
import type {BuildType} from './version-utils';
*/

const {getVersion, validateBuildType} = require('./version-utils');
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
  Usage: node ./utils/scripts/hermes/get-hermes-version.js [OPTIONS]

  Generates and outputs version string for the given build type.

  Options:
    --build-type       One of ['dry-run', 'release'].
    `);
    return;
  }

  if (!validateBuildType(buildType)) {
    throw new Error(`Unsupported build type: ${buildType}`);
  }

  const version = await getVersion(buildType);
  console.log(version);
}

if (require.main === module) {
  void main();
}
