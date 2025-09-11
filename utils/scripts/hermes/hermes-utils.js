/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

"use strict";

const { spawnSync } = require("child_process");
const fs = require("fs");
const os = require("os");
const path = require("path");

/**
 * Delegate execution to the supplied command.
 *
 * @param command Path to the command.
 * @param args Array of arguments pass to the command.
 * @param options child process options.
 */
function delegateSync(
  command /*: string */,
  args /*: (Array<string> | child_process$spawnSyncOpts) */,
  options /*: ?child_process$spawnSyncOpts */
) {
  return spawnSync(command, args, { stdio: "inherit", ...options });
}

function getHermesPrebuiltArtifactsTarballName(
  buildType /*:: ?: string */
) /*: string */ {
  if (buildType == null) {
    throw Error("Did not specify build type.");
  }
  return `hermes-ios-${buildType.toLowerCase()}.tar.gz`;
}

/**
 * Creates a tarball with the contents of the supplied directory.
 */
function createTarballFromDirectory(
  directory /*: string */,
  filename /*: string */
) {
  const args = ["-C", directory, "-czvf", filename, "."];
  delegateSync("tar", args);
}

function createHermesPrebuiltArtifactsTarball(
  hermesDir /*: string */,
  buildType /*: string */,
  tarballOutputDir /*: string */,
  excludeDebugSymbols /*: boolean */
) /*: string */ {
  validateHermesFrameworksExist(path.join(hermesDir, "destroot"));

  if (!fs.existsSync(tarballOutputDir)) {
    fs.mkdirSync(tarballOutputDir, { recursive: true });
  }

  let tarballTempDir;
  try {
    tarballTempDir = fs.mkdtempSync(
      path.join(os.tmpdir(), "hermes-engine-destroot-")
    );

    let args = ["-a"];
    if (excludeDebugSymbols) {
      args.push("--exclude=dSYMs/");
      args.push("--exclude=*.dSYM/");
    }
    args.push("./destroot");
    args.push(tarballTempDir);
    delegateSync("rsync", args, {
      cwd: hermesDir,
    });
    if (fs.existsSync(path.join(hermesDir, "LICENSE"))) {
      delegateSync("cp", ["LICENSE", tarballTempDir], { cwd: hermesDir });
    }
  } catch (error) {
    throw new Error(`Failed to copy destroot to tempdir: ${error}`);
  }

  const tarballFilename = path.join(
    tarballOutputDir,
    getHermesPrebuiltArtifactsTarballName(buildType)
  );

  try {
    createTarballFromDirectory(tarballTempDir, tarballFilename);
  } catch (error) {
    throw new Error(`[Hermes] Failed to create tarball: ${error}`);
  }

  if (!fs.existsSync(tarballFilename)) {
    throw new Error(
      `Tarball creation failed, could not locate tarball at ${tarballFilename}`
    );
  }

  return tarballFilename;
}

function validateHermesFrameworksExist(destrootDir /*: string */) {
  if (
    !fs.existsSync(
      path.join(destrootDir, "Library/Frameworks/macosx/hermesvm.framework")
    )
  ) {
    throw new Error(
      "Error: Hermes macOS Framework not found. Are you sure Hermes has been built?"
    );
  }
  if (
    !fs.existsSync(
      path.join(
        destrootDir,
        "Library/Frameworks/universal/hermesvm.xcframework"
      )
    )
  ) {
    throw new Error(
      "Error: Hermes iOS XCFramework not found. Are you sure Hermes has been built?"
    );
  }
}

module.exports = {
  createHermesPrebuiltArtifactsTarball,
};
