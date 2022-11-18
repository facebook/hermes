// @nolint
// This file is a modified version of the rimraf module on npm. It has been
// modified in the following ways:
// - Use of the assert module has been replaced with core's error system.
// - All code related to the glob dependency has been removed.
// - Bring your own custom fs module is not currently supported.
// - Some basic code cleanup.
'use strict';

var _primordials = primordials,
    ArrayPrototypeForEach = _primordials.ArrayPrototypeForEach,
    Promise = _primordials.Promise,
    SafeSet = _primordials.SafeSet;

var _require = require('buffer'),
    Buffer = _require.Buffer;

var fs = require('fs');

var chmod = fs.chmod,
    chmodSync = fs.chmodSync,
    lstat = fs.lstat,
    lstatSync = fs.lstatSync,
    readdir = fs.readdir,
    readdirSync = fs.readdirSync,
    rmdir = fs.rmdir,
    rmdirSync = fs.rmdirSync,
    stat = fs.stat,
    statSync = fs.statSync,
    unlink = fs.unlink,
    unlinkSync = fs.unlinkSync;

var _require2 = require('path'),
    sep = _require2.sep;

var _require3 = require('timers'),
    setTimeout = _require3.setTimeout;

var _require4 = require('internal/util'),
    sleep = _require4.sleep;

var notEmptyErrorCodes = new SafeSet(['ENOTEMPTY', 'EEXIST', 'EPERM']);
var retryErrorCodes = new SafeSet(['EBUSY', 'EMFILE', 'ENFILE', 'ENOTEMPTY', 'EPERM']);
var isWindows = process.platform === 'win32';
var epermHandler = isWindows ? fixWinEPERM : _rmdir;
var epermHandlerSync = isWindows ? fixWinEPERMSync : _rmdirSync;
var readdirEncoding = 'buffer';
var separator = Buffer.from(sep);

function rimraf(path, options, callback) {
  var retries = 0;

  _rimraf(path, options, function CB(err) {
    if (err) {
      if (retryErrorCodes.has(err.code) && retries < options.maxRetries) {
        retries++;
        var delay = retries * options.retryDelay;
        return setTimeout(_rimraf, delay, path, options, CB);
      } // The file is already gone.


      if (err.code === 'ENOENT') err = null;
    }

    callback(err);
  });
}

function _rimraf(path, options, callback) {
  // SunOS lets the root user unlink directories. Use lstat here to make sure
  // it's not a directory.
  lstat(path, function (err, stats) {
    if (err) {
      if (err.code === 'ENOENT') return callback(null); // Windows can EPERM on stat.

      if (isWindows && err.code === 'EPERM') return fixWinEPERM(path, options, err, callback);
    } else if (stats.isDirectory()) {
      return _rmdir(path, options, err, callback);
    }

    unlink(path, function (err) {
      if (err) {
        if (err.code === 'ENOENT') return callback(null);
        if (err.code === 'EISDIR') return _rmdir(path, options, err, callback);

        if (err.code === 'EPERM') {
          return epermHandler(path, options, err, callback);
        }
      }

      return callback(err);
    });
  });
}

function fixWinEPERM(path, options, originalErr, callback) {
  chmod(path, 438, function (err) {
    if (err) return callback(err.code === 'ENOENT' ? null : originalErr);
    stat(path, function (err, stats) {
      if (err) return callback(err.code === 'ENOENT' ? null : originalErr);
      if (stats.isDirectory()) _rmdir(path, options, originalErr, callback);else unlink(path, callback);
    });
  });
}

function _rmdir(path, options, originalErr, callback) {
  rmdir(path, function (err) {
    if (err) {
      if (notEmptyErrorCodes.has(err.code)) return _rmchildren(path, options, callback);
      if (err.code === 'ENOTDIR') return callback(originalErr);
    }

    callback(err);
  });
}

function _rmchildren(path, options, callback) {
  var pathBuf = Buffer.from(path);
  readdir(pathBuf, readdirEncoding, function (err, files) {
    if (err) return callback(err);
    var numFiles = files.length;
    if (numFiles === 0) return rmdir(path, callback);
    var done = false;
    ArrayPrototypeForEach(files, function (child) {
      var childPath = Buffer.concat([pathBuf, separator, child]);
      rimraf(childPath, options, function (err) {
        if (done) return;

        if (err) {
          done = true;
          return callback(err);
        }

        numFiles--;
        if (numFiles === 0) rmdir(path, callback);
      });
    });
  });
}

function rimrafPromises(path, options) {
  return new Promise(function (resolve, reject) {
    rimraf(path, options, function (err) {
      if (err) return reject(err);
      resolve();
    });
  });
}

function rimrafSync(path, options) {
  var stats;

  try {
    stats = lstatSync(path);
  } catch (err) {
    if (err.code === 'ENOENT') return; // Windows can EPERM on stat.

    if (isWindows && err.code === 'EPERM') fixWinEPERMSync(path, options, err);
  }

  try {
    var _stats;

    // SunOS lets the root user unlink directories.
    if ((_stats = stats) !== null && _stats !== void 0 && _stats.isDirectory()) _rmdirSync(path, options, null);else _unlinkSync(path, options);
  } catch (err) {
    if (err.code === 'ENOENT') return;
    if (err.code === 'EPERM') return epermHandlerSync(path, options, err);
    if (err.code !== 'EISDIR') throw err;

    _rmdirSync(path, options, err);
  }
}

function _unlinkSync(path, options) {
  var tries = options.maxRetries + 1;

  for (var i = 1; i <= tries; i++) {
    try {
      return unlinkSync(path);
    } catch (err) {
      // Only sleep if this is not the last try, and the delay is greater
      // than zero, and an error was encountered that warrants a retry.
      if (retryErrorCodes.has(err.code) && i < tries && options.retryDelay > 0) {
        sleep(i * options.retryDelay);
      } else if (err.code === 'ENOENT') {
        // The file is already gone.
        return;
      } else if (i === tries) {
        throw err;
      }
    }
  }
}

function _rmdirSync(path, options, originalErr) {
  try {
    rmdirSync(path);
  } catch (err) {
    if (err.code === 'ENOENT') return;

    if (err.code === 'ENOTDIR') {
      throw originalErr || err;
    }

    if (notEmptyErrorCodes.has(err.code)) {
      // Removing failed. Try removing all children and then retrying the
      // original removal. Windows has a habit of not closing handles promptly
      // when files are deleted, resulting in spurious ENOTEMPTY failures. Work
      // around that issue by retrying on Windows.
      var pathBuf = Buffer.from(path);
      ArrayPrototypeForEach(readdirSync(pathBuf, readdirEncoding), function (child) {
        var childPath = Buffer.concat([pathBuf, separator, child]);
        rimrafSync(childPath, options);
      });
      var tries = options.maxRetries + 1;

      for (var i = 1; i <= tries; i++) {
        try {
          return fs.rmdirSync(path);
        } catch (err) {
          // Only sleep if this is not the last try, and the delay is greater
          // than zero, and an error was encountered that warrants a retry.
          if (retryErrorCodes.has(err.code) && i < tries && options.retryDelay > 0) {
            sleep(i * options.retryDelay);
          } else if (err.code === 'ENOENT') {
            // The file is already gone.
            return;
          } else if (i === tries) {
            throw err;
          }
        }
      }
    }

    throw originalErr || err;
  }
}

function fixWinEPERMSync(path, options, originalErr) {
  try {
    chmodSync(path, 438);
  } catch (err) {
    if (err.code === 'ENOENT') return;
    throw originalErr;
  }

  var stats;

  try {
    stats = statSync(path, {
      throwIfNoEntry: false
    });
  } catch (_unused) {
    throw originalErr;
  }

  if (stats === undefined) return;
  if (stats.isDirectory()) _rmdirSync(path, options, originalErr);else _unlinkSync(path, options);
}

module.exports = {
  rimraf: rimraf,
  rimrafPromises: rimrafPromises,
  rimrafSync: rimrafSync
};
