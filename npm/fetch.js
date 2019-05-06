#!/usr/bin/env node

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
    readable
      .on('error', reject)
      .on('finish', resolve)
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

  const response = (await eventsOnce(req, 'response'))[0];
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
  const url = "https://api.github.com/repos/facebook/hermes/releases/assets/12341515";
  const tarball = {
    name: "hermes-cli-darwin-v" + releaseVersion + ".tar.gz",
    digest: "4f9dd949106e8c116a5ae184fb3e6def268fa9687b17128ba6806f5cb272d089"
  };
  const files = [
    {
      name: "osx-bin/hermes",
      digest: "f20d9c725de2a13c2034cc1ecb3e539e4fd733108b413db42dad4f4da53282bd"
    },
    {
      name: "osx-bin/hermes-repl",
      digest: "b50f1c87d5882b2e1d47434c157a1b269aba645dc6f7aa21bf456a801318d229"
    }
  ];
  const destdir = "osx-bin";

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvRuntimeAndroid() {
  const url = "https://api.github.com/repos/facebook/hermes/releases/assets/12344340";
  const tarball = {
    name: "hermes-runtime-android-v" + releaseVersion + ".tar.gz",
    digest: "33cef5ef42268948f4fab584fac85c5f52258c3ae2a35faf16ac9d5dfa770b33"
  };
  const files = [
    {
      name: "android/hermes-debug.aar",
      digest: "808cd855b0110b52f9b5c42ffacfdd0de249c2c20012ce98b07850bd26fe654b"
    },
    {
      name: "android/hermes-release.aar",
      digest: "cb7b142e6e2b79b0b98c84c84df20282791240b17d6f024dc24db8d053bdf5e3"
    }
  ];
  const destdir = "android";

  await fetchUnpackVerify(tarball, files, url, destdir);
}

async function fuvAll() {
  await fuvCliDarwin();
  await fuvRuntimeAndroid();
}

fuvAll()
  .catch(err => {
    console.error(err);
    process.exitCode = 1;
  });
