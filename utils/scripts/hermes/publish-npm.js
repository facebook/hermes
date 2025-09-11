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
import type { ShellString } from 'shelljs';
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
 * This script prepares a release version of hermes-compiler-v2 and may publish to NPM.
 * It is supposed to run in CI environment, not on a developer's machine.
 *
 * For a dry run, this script will:
 *  * Version the same as for the release
 *  * publish with a `--dry-run` flag
 *
 * For a release run, this script will:
 *  * Version the release by the version specified in the CMakeLists.txt
 *  * Publish to npm
 *     * without setting a tag
 */
async function main() {
  const argv = yargs
    .option('t', {
      alias: 'builtType',
      describe: 'The type of build you want to perform.',
      choices: ['dry-run', 'release'],
      default: 'dry-run',
    })
    .strict().argv;

  // $FlowFixMe[prop-missing]
  const buildType = argv.builtType;

  if (!validateBuildType(buildType)) {
    throw new Error(`Unsupported build type: ${buildType}`);
  }

  const result = await publishNpm(buildType);

  if (result && result.code) {
    const version = await getVersion(buildType);
    throw new Error(`Failed to publish hermes-compiler@${version}`);
  }
}

async function publishNpm(
  buildType /*: BuildType */,
) /*: Promise<ShellString | null> */ {
  let dryRunFlag = '';

  const version = await getVersion(buildType);
  await updatePackageJsonVersion(version);
  if (buildType === 'dry-run') {
    dryRunFlag = ` --dry-run`;
  }

  const otp = process.env.NPM_CONFIG_OTP;
  const otpFlag = otp != null ? ` --otp ${otp}` : '';
  const packagePath = path.join(REPO_ROOT, 'npm', 'hermes-compiler-v2');
  const options = {cwd: packagePath};

  return exec(`npm publish${otpFlag}${dryRunFlag}`, options);
}

if (require.main === module) {
  void main();
}
