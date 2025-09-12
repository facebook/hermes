/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

/*::
export type BuildType = 'dry-run' | 'release' | 'commitly';
export type Version = {
    version: string,
    major: string,
    minor: string,
    patch: string,
}
*/

const {ANDROID_DIR, REPO_ROOT} = require('./consts');
const {getCurrentCommit} = require('./scm-utils');
const {promises: fs} = require('fs');
const path = require('path');

const GRADLE_FILE_PATH = path.join(ANDROID_DIR, 'gradle.properties');

function validateBuildType(
  buildType /*: string */,
  // $FlowFixMe[incompatible-type-guard]
) /*: buildType is BuildType */ {
  const validBuildTypes = new Set(['release', 'dry-run', 'commitly']);

  // $FlowFixMe[incompatible-return]
  // $FlowFixMe[incompatible-type-guard]
  return validBuildTypes.has(buildType);
}

// commitly version: 0.x.y-commitly-<YY:mm:DDThh:MM>-<commit-sha>
async function getVersion(buildType /*: BuildType */) /*: Promise<string> */ {
  const currentCommit = getCurrentCommit();
  const shortCommit = currentCommit.slice(0, 9);

  const mainVersion = await getMainVersion();
  if (['commitly', 'dry-run'].includes(buildType)) {
    const date = new Date();
    const hours = date.getHours().toString().padStart(2, '0');
    const minutes = date.getMinutes().toString().padStart(2, '0');
    const dateIdentifier =
      date.toISOString().slice(0, -14).replace(/[-]/g, '') +
      `${hours}${minutes}`;
    return `${mainVersion}-commitly-${dateIdentifier}-${shortCommit}`;
  }

  // release
  return mainVersion;
}

async function getMainVersion() /*: Promise<string> */ {
  const hermesCompilerPackageJsonPath = path.join(
    REPO_ROOT,
    'npm',
    'hermes-compiler',
    'package.json',
  );
  const packageJson = JSON.parse(
    await fs.readFile(hermesCompilerPackageJsonPath, 'utf-8'),
  );
  return packageJson.version;
}

async function updateGradlePropertiesFile(
  version /*: string */,
) /*: Promise<void> */ {
  const contents = await fs.readFile(GRADLE_FILE_PATH, 'utf-8');

  return fs.writeFile(
    GRADLE_FILE_PATH,
    contents.replace(/^VERSION_NAME=.*/, `VERSION_NAME=${version}`),
  );
}

module.exports = {
  validateBuildType,
  getVersion,
  updateGradlePropertiesFile,
};
