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
export type BuildType = 'dry-run' | 'release';
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

const CMAKE_FILE_PATH = `${REPO_ROOT}/CMakeLists.txt`;
const GRADLE_FILE_PATH = path.join(ANDROID_DIR, 'gradle.properties');
const CMAKE_VERSION_REGEX =
  /project\(Hermes\s+VERSION\s+(\d+\.\d+\.\d+)\s+LANGUAGES\s+C\s+CXX\)/;

function validateBuildType(
  buildType /*: string */,
  // $FlowFixMe[incompatible-type-guard]
) /*: buildType is BuildType */ {
  const validBuildTypes = new Set(['release', 'dry-run']);

  // $FlowFixMe[incompatible-return]
  // $FlowFixMe[incompatible-type-guard]
  return validBuildTypes.has(buildType);
}

async function getVersion(buildType /*: BuildType */) /*: Promise<string> */ {
  const mainVersion = await getMainVersion();

  if (buildType === 'dry-run') {
    const currentCommit = getCurrentCommit();
    const shortCommit = currentCommit.slice(0, 9);
    return `${mainVersion}-${shortCommit}`;
  }

  return mainVersion;
}

async function getMainVersion() /*: Promise<string> */ {
  const cmakeContent = await fs.readFile(CMAKE_FILE_PATH, 'utf8');
  const versionMatch = extractMatchIfValid(cmakeContent);
  const [, version] = versionMatch;
  return version;
}

function extractMatchIfValid(cmakeContent /*: string */) {
  const match = cmakeContent.match(CMAKE_VERSION_REGEX);
  if (!match) {
    throw new Error('Could not find version in CMakeLists.txt');
  }
  return match;
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

async function updatePackageJsonVersion(
  version /*: string */,
) /*: Promise<void> */ {
  const hermesCompilerPackageJsonPath = path.join(
    REPO_ROOT,
    'npm',
    'hermes-compiler-v2',
    'package.json',
  );
  const packageJson = JSON.parse(
    await fs.readFile(hermesCompilerPackageJsonPath, 'utf-8'),
  );
  packageJson.version = version;
  await fs.writeFile(
    'package.json',
    JSON.stringify(packageJson, null, 2) + '\n',
  );
}

module.exports = {
  validateBuildType,
  getVersion,
  updateGradlePropertiesFile,
  updatePackageJsonVersion,
};
