/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const shell = require('shelljs');
const mainpkg = require('./package.json');
const fs = require('fs');

const commandLineArgs = require('command-line-args');
const optionDefinitions = [
  { name: 'dev', type: Boolean }
];
const options = commandLineArgs(optionDefinitions);

// Abort script on any failure from any shelljs invocation
shell.config.fatal = true;

// shelljs has no mktemp, so kludge our own
shell.mktemp = function() {
  let dir = shell.tempdir() + "/" + (Date.now() + Math.random());
  shell.mkdir(dir);
  return dir;
}

function createNpm(dirname) {
  if(!shell.test("-e", dirname + "/package.json")) {
    throw new Error("Can't find package.json in " + dirname);
  }
  const here = shell.pwd();
  const tmpdir = shell.mktemp("-d");

  shell.cp("-R", "*", tmpdir);
  shell.cp(dirname + "/*", tmpdir);
  shell.cd(tmpdir);
  shell.sed("-i", "%VERSION%", mainpkg.version, "package.json");
  shell.exec("${YARN:-yarn} pack");
  verifyManifest(require(`${here}/${dirname}/package.json`));
  shell.cp(dirname + "*.tgz", here);
  shell.cd(here);
  shell.rm("-rf", tmpdir);
}

function verifyManifest(manifest) {
  if (options.dev) return;

  // TODO: Handle globs
  const files = manifest.files.filter(s => s.indexOf("*") === -1);
  for (const file of files) {
    if (!fs.existsSync(file)) {
      throw `File missing from manifest: ${file}`
    }
  }
}

const npms = shell.ls("*/package.json").map(c => c.replace(/\/.*/, ''));
npms.forEach(createNpm);
