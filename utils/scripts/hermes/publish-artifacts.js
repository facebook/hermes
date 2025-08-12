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
  validateBuildType,
  updateGradlePropertiesFile,
} = require('./version-utils');
const {echo, exec, exit} = require('shelljs');
const yargs = require('yargs');

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

  await publishArtifacts(buildType);
}

async function publishArtifacts(buildType /*: BuildType */) {
  if (buildType === 'dry-run') {
    console.log('Skipping publishing artifacts because --dry-run is set.');
    return;
  }

  const version = getVersion(buildType);

  updateGradlePropertiesFile(version);
  await publishAndroidArtifactsToMaven(version, buildType);
  await publishExternalArtifactsToMaven(version, buildType);
}

function publishAndroidArtifactsToMaven(
  releaseVersion /*: string */,
  buildType /*: BuildType */,
) {
  // We want to gate ourselves against accidentally publishing a 1.x or a 1000.x on
  // maven central which will break the semver for our artifacts.
  if (
    buildType === 'release' &&
    releaseVersion.startsWith('0.') &&
    !releaseVersion.startsWith('0.0.0')
  ) {
    // -------- For stable releases, we also need to close and release the staging repository.
    if (
      exec(
        './android/gradlew publishAndroidToSonatype closeSonatypeStagingRepository',
      ).code
    ) {
      echo(
        'Failed to close and release the staging repository on Sonatype (Maven Central) for Android artifacts',
      );
      exit(1);
    }
  } else {
    echo(
      'Nothing to do as this is not a stable release - Nightlies Android artifacts are published by build_android',
    );
  }

  echo('Finished publishing Android artifacts to Maven Central');
}

function publishExternalArtifactsToMaven(
  releaseVersion /*: string */,
  buildType /*: BuildType */,
) {
  // We want to gate ourselves against accidentally publishing a 1.x or a 1000.x on
  // maven central which will break the semver for our artifacts.
  if (
    buildType === 'release' &&
    releaseVersion.startsWith('0.') &&
    !releaseVersion.startsWith('0.0.0')
  ) {
    // -------- For stable releases, we do the publishing and close the staging repository.
    // This can't be done earlier in build_android because this artifact are partially built by the iOS jobs.
    if (
      exec(
        './android/gradlew :ios-artifacts:publishToSonatype closeSonatypeStagingRepository',
      ).code
    ) {
      echo(
        'Failed to close and release the staging repository on Sonatype (Maven Central) for external artifacts',
      );
      exit(1);
    }
  } else {
    const isSnapshot = buildType === 'commitly';
    // -------- For nightly releases, we only need to publish the snapshot to Sonatype snapshot repo.
    if (
      exec(
        './android/gradlew :ios-artifacts:publishToSonatype -PisSnapshot=' +
          isSnapshot.toString(),
      ).code
    ) {
      echo('Failed to publish external artifacts to Sonatype (Maven Central)');
      exit(1);
    }
  }

  echo('Finished publishing external artifacts to Maven Central');
}

if (require.main === module) {
  void main();
}
