#!/usr/bin/env node
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//

const crypto = require('crypto');
const events = require('events');
const fs = require('fs');
const request = require('request');
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

function releaseUrlFor(name) {
  return `https://github.com/facebook/hermes/releases/download/v${releaseVersion}/hermes-${name}-v${releaseVersion}.tar.gz`;
}

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

async function verifyDigest(file, options = {}) {
  const hasher = crypto.createHash('sha256');
  hasher.setEncoding('hex');

  // This has to be before the pipeline to avoid dropping data
  const hashDigest = readAll(hasher);

  try {
    await pipeline(
      fs.createReadStream(file.name),
      hasher);
  } catch (err) {
    if (err.code !== "ENOENT" || options.hardError) {
      throw err;
    }
    return false;
  }

  if (await hashDigest !== file.digest) {
    if (options.hardError) {
      throw file.name + " digest does not match";
    }
    return false;
  }

  return true;
};

async function verifyAll(files, options = {}) {
  return ((await Promise.all(files.map(file => verifyDigest(file, options))))
          .reduce((acc, cur) => acc && cur));
}

async function downloadRelease(url, dest) {
  console.log("downloading " + dest + " from " + url)
  // pause on response is needed or else data is dropped while
  // creating the events.once Promise after.  Creating the pipeline
  // automatically unpauses.
  const req = request
      .get({
        url,
        headers: {
          "Accept": "application/octet-stream",
          "User-Agent": "fetch like curl",
        }})
      .on('response', response => response.pause())

  const response = await eventsOnce(req, 'response');
  if (response.statusCode === 200) {
    // I could pipe directly to tar.extract here, but I'd rather
    // verify the hash before unpacking.
    await pipeline(response, fs.createWriteStream(dest));
  } else {
    console.error('Response status was ' + response.statusCode);
    // because process.stderr never emits finish or close, awaiting on
    // this pipeline does not work correctly.  Instead, we do the best
    // we can and await on the input ending.
    pipeline(response, process.stderr);
    await eventsOnce(response, 'end');
    throw "fetch failed";
  }
};

async function fetchUnpackVerify(tarball, files, url, destdir) {
  // In dev mode, it's up to the developer to copy the necessary
  // tarballs in place, and no hashes are checked.  This makes
  // iteration faster.

  // If the necessary files exist (hashes are not checked in dev mode), stop.
  if (options.dev) {
    try {
      await Promise.all(files.map(_ => util.promisify(fs.access)(_.name)));
      console.log(files.map(_ => _.name).join(", ") + " existing");
      return;
    } catch (err) {
      if (err.code !== "ENOENT") {
        throw err;
      }
      // fall through
    }
  } else {
    // If we have the necessary files, stop.
    if (await verifyAll(files)) {
      console.log(files.map(_ => _.name).join(", ") + " existing and verified");
      return;
    }

    // If we don't have the tarball, download it.
    if (await verifyDigest(tarball)) {
      console.log(tarball.name + " existing and verified");
    } else {
      await downloadRelease(url, tarball.name);
      await verifyDigest(tarball, { hardError: true });
      console.log(tarball.name + " fetched and verified");
    }
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
      file: tarball.name,
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
      return;
    } else {
      throw err;
    }
  }

  if (!options.dev) {
    // Verify the tarball contents
    await verifyAll(files, { hardError: true });

    console.log(tarball.name + " unpacked and verified");
  } else {
    console.log(tarball.name + " unpacked for dev build");
  }
};

async function fuvCliDarwin() {
  // When a new release is created, the hashes of all the
  // artifacts will need to be updated here before the npm is built.
  // If not, the build will fail.  TODO We could move the shasums to
  // an external file which would make it easier to cut new releases.

  const url = releaseUrlFor("cli-darwin");
  const tarball = {
    name: "hermes-cli-darwin-v" + releaseVersion + ".tar.gz",
    digest: "d90724fca1a97de39b21342a68907a4d801b638289123e36e524d180368ed3b5"
  };
  const files = [
    {
      name: "osx-bin/hermes",
      digest: "4f1002712c1a57e5d8f13f0a5c78510609f2deb71c672b5566a73971266e899a"
    },
    {
      name: "osx-bin/hermes-repl",
      digest: "e859d75405aade42720a4b7f5f5116fd6d5853ee285caff7c3739af64e36946e"
    }
  ];
  const destdir = "osx-bin";

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvCliLinux64() {
  const url = releaseUrlFor("cli-linux");
  const tarball = {
    name: "hermes-cli-linux-v" + releaseVersion + ".tar.gz",
    digest: "baba17ec252a534c65694a9ad09c8043fa21b5b081b67bcfa73301525e497ee5"
  };
  const destdir = "linux64-bin";
  const files = [
    {
      name: destdir + "/hermes",
      digest: "003ea5f67c4c450afe92f8c9b421ecfbec5a6bf08a542b24c7262b6669e2721e"
    },
    {
      name: destdir + "/hermes-repl",
      digest: "93a2475d943623a5085b46e14581e5793834ed3ddfb895075faa8c492acf9be6"
    }
  ];

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvCliWindows64() {
  const url = releaseUrlFor("cli-windows");
  const tarball = {
    name: "hermes-cli-windows-v" + releaseVersion + ".tar.gz",
    digest: "5dfa724e40d7d089f52b7b829f11302eb273b5f66d5e8b8e0d1ca89289358aae"
  };
  const destdir = "win64-bin";
  const files = [
    {
      name: destdir + "/hermes.exe",
      digest: "f477b330eaea03cc946abee5efbbec484138122931d0ab1ef4e19b563aea7ffd"
    },
    {
      name: destdir + "/hermes-repl.exe",
      digest: "27c1e523e6444e99805e4037d6ed9556cc86a76b09819c731a0a9475abfe7f6b"
    }
  ];

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvRuntimeAndroid() {
  const url = releaseUrlFor("runtime-android");
  const tarball = {
    name: "hermes-runtime-android-v" + releaseVersion + ".tar.gz",
    digest: "59ecb513b5dab6d3a3164464fcfab9878f83a109b910560727c6f80eb8d0faa7"
  };
  const files = [
    {
      name: "android/hermes-debug.aar",
      digest: "343740072195f5470399d39caba859fe844ddb24d42ca99b36a57bc2bde7ebec"
    },
    {
      name: "android/hermes-release.aar",
      digest: "dfb77f73e47c52c27ea1884a13741ef9abfc45b9f831e721e39a0a30591c0e50"
    }
  ];
  const destdir = "android";

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvAll() {
  await fuvCliDarwin();
  await fuvCliLinux64();
  await fuvCliWindows64();
  await fuvRuntimeAndroid();
}

fuvAll()
  .catch(err => {
    console.error(err);
    process.exitCode = 1;
  });
