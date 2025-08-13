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
 * This script prepares a release version of react-native and may publish to NPM.
 * It is supposed to run in CI environment, not on a developer's machine.
 *
 * For a dry run, this script will:
 *  * Version the commitly of the form `0.0.0-<commitSha>`
 *
 * For a commitly run, this script will:
 *  * Version the commitly release of the form `0.0.0-<dateIdentifier>-<commitSha>`
 *  * Publish to npm using `nightly` tag
 *
 * For a release run, this script will:
 *  * Version the release by the tag version that triggered CI
 *  * Publish to npm
 *     * using `latest` tag if commit is currently tagged `latest`
 *     * or otherwise `{major}.{minor}-stable`
 */

async function main() {
  const argv = yargs
    .option('t', {
      alias: 'builtType',
      describe: 'The type of build you want to perform.',
      choices: ['dry-run', 'commitly', 'release'],
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
  // TODO: uncomment this later
  // if (buildType === 'dry-run') {
  //   console.log('Skipping `npm publish` because --dry-run is set.');
  //   return null;
  // }

  let tagFlag = '';
  if (buildType === 'commitly') {
    // TODO: set version on commitly
    const version = await getVersion(buildType);
    await updatePackageJsonVersion(version);
    tagFlag = ` --tag nightly`;
  }

  const otp = process.env.NPM_CONFIG_OTP;
  const otpFlag = otp != null ? ` --otp ${otp}` : '';
  const packagePath = path.join(REPO_ROOT, 'npm', 'hermes-compiler');
  const options = {cwd: packagePath};

  // return exec(`npm publish${tagFlag}${otpFlag}$`, options);

  // Pack npm only for testing to see what will be inlcuded in the package
  return exec('npm pack', options);
}

if (require.main === module) {
  void main();
}
