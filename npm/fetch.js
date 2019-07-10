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
  console.log("downloading " + dest)
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
  // When a new release is created, the url and hashes of all the
  // artifacts will need to be updated here before the npm is built.
  // If not, the build will fail.  TODO We could move the shasums to
  // an external file which would make it easier to cut new releases.

  // To get this URL, do
  // curl -H 'Accept: application/json' https://api.github.com/repos/facebook/hermes/releases/tags/<releaseVersion>
  // and use the 'url' property of the asset with the desired name.
  const url = "https://api.github.com/repos/facebook/hermes/releases/assets/13575215"
  const tarball = {
    name: "hermes-cli-darwin-v" + releaseVersion + ".tar.gz",
    digest: "7f61886871180957f8bc006b857c27c240c291af04cfdd5e3544b85e341d8a15"
  };
  const files = [
    {
      name: "osx-bin/hermes",
      digest: "8c07a68cdaa77e302c93f58d4963de18d9805fb851b85a413b3db06a89109678"
    },
    {
      name: "osx-bin/hermes-repl",
      digest: "de71b9186bc252f03c63c7af6509ead2ae7492d1c65d0f5f85b46cab9878a4fa"
    }
  ];
  const destdir = "osx-bin";

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvCliLinux64() {
  const url = "https://api.github.com/repos/facebook/hermes/releases/assets/13575216"
  const tarball = {
    name: "hermes-cli-linux-v" + releaseVersion + ".tar.gz",
    digest: "3e1ddada71279d7d7c3edc6203b2d244da5278a7adf4ed8762eac3ac6b50e251"
  };
  const destdir = "linux64-bin";
  const files = [
    {
      name: destdir + "/hermes",
      digest: "33a13474fddbe615fd1e06373ef692dfd93440fc5accb5957933f4ec8cde99b2"
    },
    {
      name: destdir + "/hermes-repl",
      digest: "39f7c3e9179fcbaca763dfe0beaf29a0080023b813c53481a8c835dd0ebabc62"
    }
  ];

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvCliWindows64() {
  const url = "https://api.github.com/repos/facebook/hermes/releases/assets/13575297"
  const tarball = {
    name: "hermes-cli-windows-v" + releaseVersion + ".tar.gz",
    digest: "7850ad333e6a8a785b3a0a070037a5fab11f452585364a8b276f0b4ef381be63"
  };
  const destdir = "win64-bin";
  const files = [
    {
      name: destdir + "/hermes.exe",
      digest: "bf33ac078a4b2e01350f4cd3f7268411ac4774f8741708c84badd9c1953558da"
    },
    {
      name: destdir + "/hermes-repl.exe",
      digest: "dee3a447c06f80691a92811c26d0df7a19b168c85438c203a0218517bc618d11"
    }
  ];

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvRuntimeAndroid() {
  const url = "https://api.github.com/repos/facebook/hermes/releases/assets/13575217";
  const tarball = {
    name: "hermes-runtime-android-v" + releaseVersion + ".tar.gz",
    digest: "5a206105ac7d506d28a5dbdeb3de4294cd996fa3153615a2fbc57c7134f9534d"
  };
  const files = [
    {
      name: "android/hermes-debug.aar",
      digest: "5c4cdb5210accaf7fe180b5c5cd4fae261236c874ba97b6aa4e92cdf33a49a5b"
    },
    {
      name: "android/hermes-release.aar",
      digest: "210fad6b124dd9507fea3ce96975a1434ddef10ba93cc2784ad80fc907a7944f"
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
