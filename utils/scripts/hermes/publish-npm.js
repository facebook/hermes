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
import type { ExecOptsSync, ShellString } from 'shelljs';
*/

const {REPO_ROOT} = require('./consts');
const {
  getVersion,
  updatePackageJsonVersion,
  validateBuildType,
} = require('./version-utils');
const path = require('path');
const {exec} = require('shelljs');
const yargs = require('yargs');

/**
 * This script prepares a release version of hermes-compiler and may publish to NPM.
 * It is supposed to run in CI environment, not on a developer's machine.
 *
 * For a dry run, this script will:
 *  * Version the same as for the release
 *  * publish with a `--dry-run` flag
 *
 * For a release run, this script will:
 *  * Version the release by the version specified in the CMakeLists.txt
 *  * Publish to npm
 *     * optionally updating the `latest-v1` tag
 */
async function main() {
  const argv = yargs
    .option('t', {
      alias: 'builtType',
      describe: 'The type of build you want to perform.',
      choices: ['dry-run', 'release'],
      default: 'dry-run',
    })
    .option('update-latest-v1', {
      describe: 'Publish a release using the latest-v1 npm tag.',
      type: 'boolean',
      default: false,
    })
    .strict().argv;

  // $FlowFixMe[prop-missing]
  const buildType = argv.builtType;
  // $FlowFixMe[prop-missing]
  const updateLatestV1 = argv.updateLatestV1;

  if (!validateBuildType(buildType)) {
    throw new Error(`Unsupported build type: ${buildType}`);
  }

  const result = await publishNpm(buildType, updateLatestV1);

  if (result && result.code) {
    const version = await getVersion(buildType);
    throw new Error(`Failed to publish hermes-compiler@${version}`);
  }
}

async function publishNpm(
  buildType /*: BuildType */,
  updateLatestV1 /*: boolean */,
) /*: Promise<ShellString | null> */ {
  const command = getNpmPublishCommand(buildType, updateLatestV1);
  const packagePath = path.join(REPO_ROOT, 'npm', 'hermes-compiler');
  const options /*: ExecOptsSync */ = {cwd: packagePath};

  return exec(command, options);
}

function getNpmPublishCommand(
  buildType /*: BuildType */,
  updateLatestV1 /*: boolean */,
) /*: string */ {
  // The tag is always explicit. npm >= 11 refuses to apply `latest`
  // implicitly when a higher version is already published.
  let publishFlags = ` --tag ${updateLatestV1 ? 'latest-v1' : 'latest'}`;

  if (buildType === 'dry-run') {
    publishFlags += ` --dry-run`;
  }

  return `npm publish${publishFlags}`;
}

if (require.main === module) {
  void main();
}
