#!/usr/bin/env node
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const crypto = require('crypto');
const events = require('events');
const fs = require('fs');
const stream = require('stream');
const tar = require('tar');
const util = require('util');
const commandLineArgs = require('command-line-args');

const npmjson = require('./package.json');
const releaseVersion = npmjson.version;

const optionDefinitions = [
  { name: 'dev', type: Boolean }
];
const options = commandLineArgs(optionDefinitions);

// This is a limited replacement for util.promisify(stream.pipeline),
// because node 8 doesn't have stream.pipeline.
function pipeline(readable, writable) {
  return new Promise((resolve, reject) => {
    writable
      .on('finish', resolve)
      .on('error', reject);
    readable
      .on('error', reject)
      .pipe(writable);
  });
}

// This is a limited replacement for events.once, because node 8
// doesn't have it.

function eventsOnce(stream, ev) {
  return new Promise((resolve, reject) => {
    stream.once(ev, resolve);
  });
}

async function readAll(readable) {
  var ret = undefined;
  readable.on('data', (d) => ret = (ret || "") + d.toString());
  await eventsOnce(readable, 'end');
  return ret;
};

async function verifyDigest(file) {
  const hasher = crypto.createHash('sha256');
  hasher.setEncoding('hex');

  var sumFile = fs.readFileSync(file + ".sha256").toString();
  var expected = sumFile.split(" ")[0].toLowerCase();

  // This has to be before the pipeline to avoid dropping data
  const hashDigest = readAll(hasher);

  await pipeline(
    fs.createReadStream(file),
    hasher);

  var actual = await hashDigest;

  if (actual !== expected) {
    throw `${file} has digest ${actual}, expected ${expected}`
  }
};

async function unpack(tarball, destdir) {
  // In dev mode, no hashes are checked.
  if (!options.dev) {
    await verifyDigest(tarball);
  }

  try {
    await util.promisify(fs.mkdir)(destdir, {recursive:true});
  } catch (err) {
    // node 8 doesn't have the recursive option, so ignore EEXIST
    if (err.code !== "EEXIST") {
      throw err;
    }
  }

  try {
    // Unpack the tarball
    const outputs = [];
    await tar.extract({
      file: tarball,
      cwd: destdir,
      onentry: entry => {
        console.log("unpacking " + entry.path);
        outputs.push(entry.path);
      }});
  } catch (err) {
    if (options.dev) {
      // In dev mode, it's likely cli tarballs for other platforms will
      // be missing.  Just log it and move on.
      console.warn("Ignoring missing tarball in dev mode", err);
      return false;
    } else {
      throw err;
    }
  }
  return true;
};

async function unpackAll(files) {
  var results = await Promise.all(files.map(x => unpack(x.name, x.dest)));
  if (!results.some(c => c)) {
    var names = inputs.map(c => c.name).join(", ");
    throw "No tarballs found. Please build or download at least one of: " + names;
  }
}

var inputs = [
  {
    name: "hermes-runtime-android-v" + releaseVersion + ".tar.gz",
    dest: "android"
  },
  {
    name: "hermes-cli-windows-v" + releaseVersion + ".tar.gz",
    dest: "win64-bin"
  },
  {
    name: "hermes-cli-linux-v" + releaseVersion + ".tar.gz",
    dest: "linux64-bin"
  },
  {
    name: "hermes-cli-darwin-v" + releaseVersion + ".tar.gz",
    dest: "osx-bin"
  }
]

unpackAll(inputs)
  .catch(err => {
    console.error(err);
    process.exitCode = 1;
  });
