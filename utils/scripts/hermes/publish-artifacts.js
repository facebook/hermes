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

const {
  getVersion,
  updateGradlePropertiesFile,
  validateBuildType,
} = require('./version-utils');
const {echo, exec, exit, popd, pushd} = require('shelljs');
const yargs = require('yargs');

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

  await publishArtifacts(buildType);
}

async function publishArtifacts(buildType /*: BuildType */) {
  if (buildType === 'dry-run') {
    console.log('Skipping publishing artifacts because --dry-run is set.');
    return;
  }

  const version = await getVersion(buildType);
  await updateGradlePropertiesFile(version);

  pushd('android');
  await publishAndroidArtifactsToMaven(version, buildType);
  await publishExternalArtifactsToMaven(version, buildType);
  popd();
}

function publishAndroidArtifactsToMaven(
  releaseVersion /*: string */,
  buildType /*: BuildType */,
) {
  if (buildType === 'release') {
    // -------- For stable releases, we also need to close and release the staging repository.
    if (
      exec(
        './gradlew publishAndroidOnlyToSonatype closeAndReleaseSonatypeStagingRepository',
      ).code
    ) {
      echo(
        'Failed to close and release the staging repository on Sonatype (Maven Central) for Android artifacts',
      );
      exit(1);
    }
  } else {
    echo('Nothing to do as this is not a stable release');
  }

  echo('Finished publishing Android artifacts to Maven Central');
}

function publishExternalArtifactsToMaven(
  releaseVersion /*: string */,
  buildType /*: BuildType */,
) {
  if (buildType === 'release') {
    // -------- For stable releases, we do the publishing and close the staging repository.
    // This can't be done earlier in build_android because this artifact are partially built by the iOS jobs.
    if (
      exec(
        './gradlew :ios-artifacts:publishToSonatype closeAndReleaseSonatypeStagingRepository',
      ).code
    ) {
      echo(
        'Failed to close and release the staging repository on Sonatype (Maven Central) for external artifacts',
      );
      exit(1);
    }
  } else {
    echo('Nothing to do as this is not a stable release');
  }

  echo('Finished publishing external artifacts to Maven Central');
}

if (require.main === module) {
  void main();
}
