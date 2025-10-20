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
 *  * Version the commitly release of the form `0.x.y-commitly-<YY:mm:DDThh:MM>-<commit-sha>`
 *  * publish with a `--dry-run` flag
 *
 * For a commitly run, this script will:
 *  * Version the commitly release of the form `0.x.y-commitly-<YY:mm:DDThh:MM>-<commit-sha>`
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
    .option('v', {
      alias: 'hermesVersion',
      describe: 'Use a specific version instead of generating one.',
      type: 'string',
    })
    .strict().argv;

  // $FlowFixMe[prop-missing]
  const buildType = argv.builtType;
  // $FlowFixMe[prop-missing]
  const hermesVersion = argv.hermesVersion;

  if (!validateBuildType(buildType)) {
    throw new Error(`Unsupported build type: ${buildType}`);
  }

  if (hermesVersion && buildType !== 'release') {
    await updatePackageJsonVersion(hermesVersion);
  }

  const result = await publishNpm(buildType);

  if (result && result.code) {
    const version = hermesVersion ? hermesVersion : await getVersion(buildType);
    throw new Error(`Failed to publish hermes-compiler@${version}`);
  }
}

async function publishNpm(
  buildType /*: BuildType */,
) /*: Promise<ShellString | null> */ {
  let tagFlag = '';

  if (buildType === 'dry-run') {
    tagFlag = ` --tag nightly --dry-run`;
  } else if (buildType === 'commitly') {
    tagFlag = ` --tag nightly`;
  } else if (buildType === 'release') {
    tagFlag = ` --tag latest-v0`;
  }

  const packagePath = path.join(REPO_ROOT, 'npm', 'hermes-compiler');
  const options /*: ExecOptsSync */ = {cwd: packagePath};

  return exec(`npm publish${tagFlag}`, options);
}

if (require.main === module) {
  void main();
}
