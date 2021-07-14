// @nolint
// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.
// Maintainers, keep in mind that ES1-style octal literals (`0666`) are not
// allowed in strict mode. Use ES6-style octal literals instead (`0o666`).
'use strict'; // When using FSReqCallback, make sure to create the object only *after* all
// parameter validation has happened, so that the objects are not kept in memory
// in case they are created but never used due to an exception.

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

var _primordials = primordials,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    BigIntPrototypeToString = _primordials.BigIntPrototypeToString,
    MathMax = _primordials.MathMax,
    Number = _primordials.Number,
    ObjectCreate = _primordials.ObjectCreate,
    ObjectDefineProperties = _primordials.ObjectDefineProperties,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    Promise = _primordials.Promise,
    ReflectApply = _primordials.ReflectApply,
    RegExpPrototypeExec = _primordials.RegExpPrototypeExec,
    SafeMap = _primordials.SafeMap,
    String = _primordials.String,
    StringPrototypeCharCodeAt = _primordials.StringPrototypeCharCodeAt,
    StringPrototypeIndexOf = _primordials.StringPrototypeIndexOf,
    StringPrototypeSlice = _primordials.StringPrototypeSlice;

var _internalBinding = internalBinding('constants'),
    constants = _internalBinding.fs;

var S_IFIFO = constants.S_IFIFO,
    S_IFLNK = constants.S_IFLNK,
    S_IFMT = constants.S_IFMT,
    S_IFREG = constants.S_IFREG,
    S_IFSOCK = constants.S_IFSOCK,
    F_OK = constants.F_OK,
    R_OK = constants.R_OK,
    W_OK = constants.W_OK,
    X_OK = constants.X_OK,
    O_WRONLY = constants.O_WRONLY,
    O_SYMLINK = constants.O_SYMLINK;

var pathModule = require('path');

// var _require = require('internal/util/types'),
//     isArrayBufferView = _require.isArrayBufferView; // We need to get the statValues from the binding at the callsite since
// it's re-initialized after deserialization.


var binding = internalBinding('fs');

var _require2 = require('buffer'),
    Buffer = _require2.Buffer;

// var _require3 = require('internal/errors'),
//     aggregateTwoErrors = _require3.aggregateTwoErrors,
//     _require3$codes = _require3.codes,
//     ERR_FS_FILE_TOO_LARGE = _require3$codes.ERR_FS_FILE_TOO_LARGE,
//     ERR_INVALID_ARG_VALUE = _require3$codes.ERR_INVALID_ARG_VALUE,
//     ERR_INVALID_ARG_TYPE = _require3$codes.ERR_INVALID_ARG_TYPE,
//     ERR_FEATURE_UNAVAILABLE_ON_PLATFORM = _require3$codes.ERR_FEATURE_UNAVAILABLE_ON_PLATFORM,
//     AbortError = _require3.AbortError,
//     uvErrmapGet = _require3.uvErrmapGet,
//     uvException = _require3.uvException;

// var FSReqCallback = binding.FSReqCallback;

// var _require4 = require('internal/url'),
//     toPathIfFileURL = _require4.toPathIfFileURL;

var internalUtil = require('internal/util');

var _require5 = require('internal/fs/utils'),
    _require5$constants = _require5.constants,
    kIoMaxLength = _require5$constants.kIoMaxLength,
    kMaxUserId = _require5$constants.kMaxUserId,
    copyObject = _require5.copyObject,
//     Dirent = _require5.Dirent,
//     emitRecursiveRmdirWarning = _require5.emitRecursiveRmdirWarning,
//     getDirents = _require5.getDirents,
    getOptions = _require5.getOptions,
    getValidatedFd = _require5.getValidatedFd,
    getValidatedPath = _require5.getValidatedPath,
//     getValidMode = _require5.getValidMode,
    handleErrorFromBinding = _require5.handleErrorFromBinding,
//     nullCheck = _require5.nullCheck,
//     preprocessSymlinkDestination = _require5.preprocessSymlinkDestination,
//     Stats = _require5.Stats,
//     getStatsFromBinding = _require5.getStatsFromBinding,
//     realpathCacheKey = _require5.realpathCacheKey,
    stringToFlags = _require5.stringToFlags,
//     stringToSymlinkType = _require5.stringToSymlinkType,
//     toUnixTimestamp = _require5.toUnixTimestamp,
//     validateBufferArray = _require5.validateBufferArray,
    validateOffsetLengthRead = _require5.validateOffsetLengthRead,
//     validateOffsetLengthWrite = _require5.validateOffsetLengthWrite,
//     validatePath = _require5.validatePath,
    validatePosition = _require5.validatePosition;
//     validateRmOptions = _require5.validateRmOptions,
//     validateRmOptionsSync = _require5.validateRmOptionsSync,
//     validateRmdirOptions = _require5.validateRmdirOptions,
//     validateStringAfterArrayBufferView = _require5.validateStringAfterArrayBufferView,
//     warnOnNonPortableTemplate = _require5.warnOnNonPortableTemplate;

// var _require6 = require('internal/fs/dir'),
//     Dir = _require6.Dir,
//     opendir = _require6.opendir,
//     opendirSync = _require6.opendirSync;

// var _require7 = require('internal/constants'),
//     CHAR_FORWARD_SLASH = _require7.CHAR_FORWARD_SLASH,
//     CHAR_BACKWARD_SLASH = _require7.CHAR_BACKWARD_SLASH;

var _require8 = require('internal/validators'),
    isUint32 = _require8.isUint32,
    parseFileMode = _require8.parseFileMode,
//     validateBoolean = _require8.validateBoolean,
    validateBuffer = _require8.validateBuffer,
//     validateCallback = _require8.validateCallback,
//     validateEncoding = _require8.validateEncoding,
//     validateFunction = _require8.validateFunction,
    validateInteger = _require8.validateInteger;

// var watchers = require('internal/fs/watchers');

// var ReadFileContext = require('internal/fs/read_file_context');

var truncateWarn = true;
var fs; // Lazy loaded

var promises = null;
var ReadStream;
var WriteStream;
var rimraf;
var rimrafSync; // These have to be separate because of how graceful-fs happens to do it's
// monkeypatching.

var FileReadStream;
var FileWriteStream;
// var isWindows = process.platform === 'win32';
// var isOSX = process.platform === 'darwin';

function showTruncateDeprecation() {
  if (truncateWarn) {
    process.emitWarning('Using fs.truncate with a file descriptor is deprecated. Please use ' + 'fs.ftruncate with a file descriptor instead.', 'DeprecationWarning', 'DEP0081');
    truncateWarn = false;
  }
}

function maybeCallback(cb) {
  validateCallback(cb);
  return cb;
} // Ensure that callbacks run in the global context. Only use this function
// for callbacks that are passed to the binding layer, callbacks that are
// invoked from JS already run in the proper scope.


function makeCallback(cb) {
  var _this = this;

  validateCallback(cb);
  return function () {
    for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments[_key];
    }

    return ReflectApply(cb, _this, args);
  };
} // Special case of `makeCallback()` that is specific to async `*stat()` calls as
// an optimization, since the data passed back to the callback needs to be
// transformed anyway.


function makeStatsCallback(cb) {
  validateCallback(cb);
  return function (err, stats) {
    if (err) return cb(err);
    cb(err, getStatsFromBinding(stats));
  };
}

var isFd = isUint32;

function isFileType(stats, fileType) {
  // Use stats array directly to avoid creating an fs.Stats instance just for
  // our internal use.
  var mode = stats[1];
  if(mode == undefined) mode = stats.mode; // New
  if (typeof mode === 'bigint') mode = Number(mode);
  return (mode & S_IFMT) === fileType;
}
/**
 * Tests a user's permissions for the file or directory
 * specified by `path`.
 * @param {string | Buffer | URL} path
 * @param {number} [mode]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function access(path, mode, callback) {
  if (typeof mode === 'function') {
    callback = mode;
    mode = F_OK;
  }

  path = getValidatedPath(path);
  mode = getValidMode(mode, 'access');
  callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.access(pathModule.toNamespacedPath(path), mode, req);
}
/**
 * Synchronously tests a user's permissions for the file or
 * directory specified by `path`.
 * @param {string | Buffer | URL} path
 * @param {number} [mode]
 * @returns {void | never}
 */


function accessSync(path, mode) {
  path = getValidatedPath(path);
  mode = getValidMode(mode, 'access');
  var ctx = {
    path: path
  };
  binding.access(pathModule.toNamespacedPath(path), mode, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Tests whether or not the given path exists.
 * @param {string | Buffer | URL} path
 * @param {(exists?: boolean) => any} callback
 * @returns {void}
 */


function exists(path, callback) {
  maybeCallback(callback);

  function suppressedCallback(err) {
    callback(err ? false : true);
  }

  try {
    fs.access(path, F_OK, suppressedCallback);
  } catch (_unused) {
    return callback(false);
  }
}

ObjectDefineProperty(exists, internalUtil.promisify.custom, {
  value: function value(path) {
    return new Promise(function (resolve) {
      return fs.exists(path, resolve);
    });
  }
}); // fs.existsSync never throws, it only returns true or false.
// Since fs.existsSync never throws, users have established
// the expectation that passing invalid arguments to it, even like
// fs.existsSync(), would only get a false in return, so we cannot signal
// validation errors to users properly out of compatibility concerns.
// TODO(joyeecheung): deprecate the never-throw-on-invalid-arguments behavior

/**
 * Synchronously tests whether or not the given path exists.
 * @param {string | Buffer | URL} path
 * @returns {boolean}
 */

function existsSync(path) {
  try {
    path = getValidatedPath(path);
  } catch (_unused2) {
    return false;
  }

  var ctx = {
    path: path
  };
  var nPath = pathModule.toNamespacedPath(path);
  binding.access(nPath, F_OK, undefined, ctx); // In case of an invalid symlink, `binding.access()` on win32
  // will **not** return an error and is therefore not enough.
  // Double check with `binding.stat()`.

  if (isWindows && ctx.errno === undefined) {
    binding.stat(nPath, false, undefined, ctx);
  }

  return ctx.errno === undefined;
}

function readFileAfterOpen(err, fd) {
  var context = this.context;

  if (err) {
    context.callback(err);
    return;
  }

  context.fd = fd;
  var req = new FSReqCallback();
  req.oncomplete = readFileAfterStat;
  req.context = context;
  binding.fstat(fd, false, req);
}

function readFileAfterStat(err, stats) {
  var context = this.context;
  if (err) return context.close(err);
  var size = context.size = isFileType(stats, S_IFREG) ? stats[8] : 0;

  if (size > kIoMaxLength) {
    err = new ERR_FS_FILE_TOO_LARGE(size);
    return context.close(err);
  }

  try {
    if (size === 0) {
      context.buffers = [];
    } else {
      context.buffer = Buffer.allocUnsafeSlow(size);
    }
  } catch (err) {
    return context.close(err);
  }

  context.read();
}

function checkAborted(signal, callback) {
  if (signal !== null && signal !== void 0 && signal.aborted) {
    callback(new AbortError());
    return true;
  }

  return false;
}
/**
 * Asynchronously reads the entire contents of a file.
 * @param {string | Buffer | URL | number} path
 * @param {{
 *   encoding?: string | null;
 *   flag?: string;
 *   signal?: AbortSignal;
 *   } | string} [options]
 * @param {(
 *   err?: Error,
 *   data?: string | Buffer
 *   ) => any} callback
 * @returns {void}
 */


function readFile(path, options, callback) {
  callback = maybeCallback(callback || options);
  options = getOptions(options, {
    flag: 'r'
  });
  var context = new ReadFileContext(callback, options.encoding);
  context.isUserFd = isFd(path); // File descriptor ownership

  if (options.signal) {
    context.signal = options.signal;
  }

  if (context.isUserFd) {
    process.nextTick(function tick(context) {
      ReflectApply(readFileAfterOpen, {
        context: context
      }, [null, path]);
    }, context);
    return;
  }

  if (checkAborted(options.signal, callback)) return;
  var flagsNumber = stringToFlags(options.flag, 'options.flag');
  path = getValidatedPath(path);
  var req = new FSReqCallback();
  req.context = context;
  req.oncomplete = readFileAfterOpen;
  binding.open(pathModule.toNamespacedPath(path), flagsNumber, 438, req);
}

function tryStatSync(fd, isUserFd) {
  var ctx = {};
  var stats = binding.fstat(fd, false, undefined, ctx);
  if (ctx.errno !== undefined && !isUserFd) {
    fs.closeSync(fd);
    throw uvException(ctx);
  }

  return stats;
}

function tryCreateBuffer(size, fd, isUserFd) {
  var threw = true;
  var buffer;
  try {
    if (size > kIoMaxLength) {
      throw new ERR_FS_FILE_TOO_LARGE(size);
    }
    buffer = Buffer.allocUnsafeSlow(size); //Changed from allocUnsafe
    threw = false;
  } finally {
    if (threw && !isUserFd) fs.closeSync(fd);
  }
  return buffer;
}

function tryReadSync(fd, isUserFd, buffer, pos, len) {
  var threw = true;
  var bytesRead;

  try {
    bytesRead = fs.readSync(fd, buffer, pos, len);
    threw = false;
  } finally {
    if (threw && !isUserFd) fs.closeSync(fd);
  }

  return bytesRead;
}
/**
 * Synchronously reads the entire contents of a file.
 * @param {string | Buffer | URL | number} path
 * @param {{
 *   encoding?: string | null;
 *   flag?: string;
 *   }} [options]
 * @returns {string | Buffer}
 */


function readFileSync(path, options) {
  options = getOptions(options, {
    flag: 'r'
  });
  var isUserFd = isFd(path); // File descriptor ownership
  var fd = isUserFd ? path : fs.openSync(path, options.flag, 438);
  var stats = tryStatSync(fd, isUserFd);
  var size = isFileType(stats, S_IFREG) ? stats.size : 0; // Change from stats[8]
  var pos = 0;
  var buffer; // Single buffer with file data

  var buffers; // List for when size is unknown

  if (size === 0) {
    buffers = [];
  } else {
    buffer = tryCreateBuffer(size, fd, isUserFd);
  }
  var bytesRead;

  if (size !== 0) {
    do {
      bytesRead = tryReadSync(fd, isUserFd, buffer, pos, size - pos);
      pos += bytesRead;
    } while (bytesRead !== 0 && pos < size);
  } else {
    do {
      // The kernel lies about many files.
      // Go ahead and try to read some bytes.
      buffer = Buffer.allocUnsafeSlow(8192);
      bytesRead = tryReadSync(fd, isUserFd, buffer, 0, 8192);

      if (bytesRead !== 0) {
        ArrayPrototypePush(buffers, buffer.slice(0, bytesRead));
      }

      pos += bytesRead;
    } while (bytesRead !== 0);
  }

  if (!isUserFd) fs.closeSync(fd);

  if (size === 0) {
    // Data was collected into the buffers list.
    buffer = Buffer.concat(buffers, pos);
  } else if (pos < size) {
    buffer = buffer.slice(0, pos);
  }

  if (options.encoding) buffer = buffer.toString(options.encoding);
  return buffer;
}

function defaultCloseCallback(err) {
  if (err != null) throw err;
}
/**
 * Closes the file descriptor.
 * @param {number} fd
 * @param {(err?: Error) => any} [callback]
 * @returns {void}
 */


function close(fd) {
  var callback = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : defaultCloseCallback;
  fd = getValidatedFd(fd);
  if (callback !== defaultCloseCallback) callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.close(fd, req);
}
/**
 * Synchronously closes the file descriptor.
 * @param {number} fd
 * @returns {void}
 */


function closeSync(fd) {
  fd = getValidatedFd(fd);
  var ctx = {};
  binding.close(fd, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Asynchronously opens a file.
 * @param {string | Buffer | URL} path
 * @param {string | number} [flags]
 * @param {string | number} [mode]
 * @param {(
 *   err?: Error,
 *   fd?: number
 *   ) => any} callback
 * @returns {void}
 */


function open(path, flags, mode, callback) {
  path = getValidatedPath(path);

  if (arguments.length < 3) {
    callback = flags;
    flags = 'r';
    mode = 438;
  } else if (typeof mode === 'function') {
    callback = mode;
    mode = 438;
  } else {
    mode = parseFileMode(mode, 'mode', 438);
  }

  var flagsNumber = stringToFlags(flags);
  callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.open(pathModule.toNamespacedPath(path), flagsNumber, mode, req);
}
/**
 * Synchronously opens a file.
 * @param {string | Buffer | URL} path
 * @param {string | number} [flags]
 * @param {string | number} [mode]
 * @returns {number}
 */


function openSync(path, flags, mode) {
  path = getValidatedPath(path);
  var flagsNumber = stringToFlags(flags);
  mode = parseFileMode(mode, 'mode', 438);
  var ctx = {
    path: path
  };
  var result = binding.open(pathModule.toNamespacedPath(path), flagsNumber, mode, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}
/**
 * Reads file from the specified `fd` (file descriptor).
 * @param {number} fd
 * @param {Buffer | TypedArray | DataView} buffer
 * @param {number} offset
 * @param {number} length
 * @param {number | bigint} position
 * @param {(
 *   err?: Error,
 *   bytesRead?: number,
 *   buffer?: Buffer
 *   ) => any} callback
 * @returns {void}
 */


function read(fd, buffer, offset, length, position, callback) {
  fd = getValidatedFd(fd);

  if (arguments.length <= 3) {
    // Assume fs.read(fd, options, callback)
    var options = {};

    if (arguments.length < 3) {
      // This is fs.read(fd, callback)
      // buffer will be the callback
      callback = buffer;
    } else {
      // This is fs.read(fd, {}, callback)
      // buffer will be the options object
      // offset is the callback
      options = buffer;
      callback = offset;
    }

    var _options = options;
    var _options$buffer = _options.buffer;
    buffer = _options$buffer === void 0 ? Buffer.alloc(16384) : _options$buffer;
    var _options$offset = _options.offset;
    offset = _options$offset === void 0 ? 0 : _options$offset;
    var _options$length = _options.length;
    length = _options$length === void 0 ? buffer.byteLength : _options$length;
    position = _options.position;
  }

  validateBuffer(buffer);
  callback = maybeCallback(callback);

  if (offset == null) {
    offset = 0;
  } else {
    validateInteger(offset, 'offset', 0);
  }

  length |= 0;

  if (length === 0) {
    return process.nextTick(function tick() {
      callback(null, 0, buffer);
    });
  }

  if (buffer.byteLength === 0) {
    throw new ERR_INVALID_ARG_VALUE('buffer', buffer, 'is empty and cannot be written');
  }

  validateOffsetLengthRead(offset, length, buffer.byteLength);
  if (position == null) position = -1;
  validatePosition(position, 'position');

  function wrapper(err, bytesRead) {
    // Retain a reference to buffer so that it can't be GC'ed too soon.
    callback(err, bytesRead || 0, buffer);
  }

  var req = new FSReqCallback();
  req.oncomplete = wrapper;
  binding.read(fd, buffer, offset, length, position, req);
}

ObjectDefineProperty(read, internalUtil.customPromisifyArgs, {
  value: ['bytesRead', 'buffer'],
  enumerable: false
});
/**
 * Synchronously reads the file from the
 * specified `fd` (file descriptor).
 * @param {number} fd
 * @param {Buffer | TypedArray | DataView} buffer
 * @param {{
 *   offset?: number;
 *   length?: number;
 *   position?: number | bigint;
 *   }} [offset]
 * @returns {number}
 */

function readSync(fd, buffer, offset, length, position) {
  fd = getValidatedFd(fd);
  validateBuffer(buffer);

  if (arguments.length <= 3) {
    // Assume fs.read(fd, buffer, options)
    var options = offset || {};
    var _options$offset2 = options.offset;
    offset = _options$offset2 === void 0 ? 0 : _options$offset2;
    var _options$length2 = options.length;
    length = _options$length2 === void 0 ? buffer.byteLength : _options$length2;
    position = options.position;
  }

  if (offset == null) {
    offset = 0;
  } else {
    validateInteger(offset, 'offset', 0);
  }
  length |= 0;

  if (length === 0) {
    return 0;
  }

  if (buffer.byteLength === 0) {
    throw new ERR_INVALID_ARG_VALUE('buffer', buffer, 'is empty and cannot be written');
  }
  validateOffsetLengthRead(offset, length, buffer.byteLength);
  if (position == null) position = -1;
  validatePosition(position, 'position');
  var ctx = {};
  var result = binding.read(fd, buffer, offset, length, position, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}
/**
 * Reads file from the specified `fd` (file descriptor)
 * and writes to an array of `ArrayBufferView`s.
 * @param {number} fd
 * @param {ArrayBufferView[]} buffers
 * @param {number} [position]
 * @param {(
 *   err?: Error,
 *   bytesRead?: number,
 *   buffers?: ArrayBufferView[];
 *   ) => any} callback
 * @returns {void}
 */


function readv(fd, buffers, position, callback) {
  function wrapper(err, read) {
    callback(err, read || 0, buffers);
  }

  fd = getValidatedFd(fd);
  validateBufferArray(buffers);
  callback = maybeCallback(callback || position);
  var req = new FSReqCallback();
  req.oncomplete = wrapper;
  if (typeof position !== 'number') position = null;
  return binding.readBuffers(fd, buffers, position, req);
}

ObjectDefineProperty(readv, internalUtil.customPromisifyArgs, {
  value: ['bytesRead', 'buffers'],
  enumerable: false
});
/**
 * Synchronously reads file from the
 * specified `fd` (file descriptor) and writes to an array
 * of `ArrayBufferView`s.
 * @param {number} fd
 * @param {ArrayBufferView[]} buffers
 * @param {number} [position]
 * @returns {number}
 */

function readvSync(fd, buffers, position) {
  fd = getValidatedFd(fd);
  validateBufferArray(buffers);
  var ctx = {};
  if (typeof position !== 'number') position = null;
  var result = binding.readBuffers(fd, buffers, position, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}
/**
 * Writes `buffer` to the specified `fd` (file descriptor).
 * @param {number} fd
 * @param {Buffer | TypedArray | DataView | string | Object} buffer
 * @param {number} [offset]
 * @param {number} [length]
 * @param {number} [position]
 * @param {(
 *   err?: Error,
 *   bytesWritten?: number;
 *   buffer?: Buffer | TypedArray | DataView
 *   ) => any} callback
 * @returns {void}
 */


function write(fd, buffer, offset, length, position, callback) {
  function wrapper(err, written) {
    // Retain a reference to buffer so that it can't be GC'ed too soon.
    callback(err, written || 0, buffer);
  }

  fd = getValidatedFd(fd);

  if (isArrayBufferView(buffer)) {
    callback = maybeCallback(callback || position || length || offset);

    if (offset == null || typeof offset === 'function') {
      offset = 0;
    } else {
      validateInteger(offset, 'offset', 0);
    }

    if (typeof length !== 'number') length = buffer.byteLength - offset;
    if (typeof position !== 'number') position = null;
    validateOffsetLengthWrite(offset, length, buffer.byteLength);

    var _req = new FSReqCallback();

    _req.oncomplete = wrapper;
    return binding.writeBuffer(fd, buffer, offset, length, position, _req);
  }

  validateStringAfterArrayBufferView(buffer, 'buffer');

  if (typeof position !== 'function') {
    if (typeof offset === 'function') {
      position = offset;
      offset = null;
    } else {
      position = length;
    }

    length = 'utf8';
  }

  var str = String(buffer);
  validateEncoding(str, length);
  callback = maybeCallback(position);
  var req = new FSReqCallback();
  req.oncomplete = wrapper;
  return binding.writeString(fd, str, offset, length, req);
}

ObjectDefineProperty(write, internalUtil.customPromisifyArgs, {
  value: ['bytesWritten', 'buffer'],
  enumerable: false
});
/**
 * Synchronously writes `buffer` to the
 * specified `fd` (file descriptor).
 * @param {number} fd
 * @param {Buffer | TypedArray | DataView | string | Object} buffer
 * @param {number} [offset]
 * @param {number} [length]
 * @param {number} [position]
 * @returns {number}
 */

function writeSync(fd, buffer, offset, length, position) {
  fd = getValidatedFd(fd);
  var ctx = {};
  var result;

  if (isArrayBufferView(buffer)) {
    if (position === undefined) position = null;

    if (offset == null) {
      offset = 0;
    } else {
      validateInteger(offset, 'offset', 0);
    }

    if (typeof length !== 'number') length = buffer.byteLength - offset;
    validateOffsetLengthWrite(offset, length, buffer.byteLength);
    result = binding.writeBuffer(fd, buffer, offset, length, position, undefined, ctx);
  } else {
    validateStringAfterArrayBufferView(buffer, 'buffer');
    validateEncoding(buffer, length);
    if (offset === undefined) offset = null;
    result = binding.writeString(fd, buffer, offset, length, undefined, ctx);
  }

  handleErrorFromBinding(ctx);
  return result;
}
/**
 * Writes an array of `ArrayBufferView`s to the
 * specified `fd` (file descriptor).
 * @param {number} fd
 * @param {ArrayBufferView[]} buffers
 * @param {number} [position]
 * @param {(
 *   err?: Error,
 *   bytesWritten?: number,
 *   buffers?: ArrayBufferView[]
 *   ) => any} callback
 * @returns {void}
 */


function writev(fd, buffers, position, callback) {
  function wrapper(err, written) {
    callback(err, written || 0, buffers);
  }

  fd = getValidatedFd(fd);
  validateBufferArray(buffers);
  callback = maybeCallback(callback || position);
  var req = new FSReqCallback();
  req.oncomplete = wrapper;
  if (typeof position !== 'number') position = null;
  return binding.writeBuffers(fd, buffers, position, req);
}

ObjectDefineProperty(writev, internalUtil.customPromisifyArgs, {
  value: ['bytesWritten', 'buffer'],
  enumerable: false
});
/**
 * Synchronously writes an array of `ArrayBufferView`s
 * to the specified `fd` (file descriptor).
 * @param {number} fd
 * @param {ArrayBufferView[]} buffers
 * @param {number} [position]
 * @returns {number}
 */

function writevSync(fd, buffers, position) {
  fd = getValidatedFd(fd);
  validateBufferArray(buffers);
  var ctx = {};
  if (typeof position !== 'number') position = null;
  var result = binding.writeBuffers(fd, buffers, position, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}
/**
 * Asynchronously renames file at `oldPath` to
 * the pathname provided as `newPath`.
 * @param {string | Buffer | URL} oldPath
 * @param {string | Buffer | URL} newPath
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function rename(oldPath, newPath, callback) {
  callback = makeCallback(callback);
  oldPath = getValidatedPath(oldPath, 'oldPath');
  newPath = getValidatedPath(newPath, 'newPath');
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.rename(pathModule.toNamespacedPath(oldPath), pathModule.toNamespacedPath(newPath), req);
}
/**
 * Synchronously renames file at `oldPath` to
 * the pathname provided as `newPath`.
 * @param {string | Buffer | URL} oldPath
 * @param {string | Buffer | URL} newPath
 * @returns {void}
 */


function renameSync(oldPath, newPath) {
  oldPath = getValidatedPath(oldPath, 'oldPath');
  newPath = getValidatedPath(newPath, 'newPath');
  var ctx = {
    path: oldPath,
    dest: newPath
  };
  binding.rename(pathModule.toNamespacedPath(oldPath), pathModule.toNamespacedPath(newPath), undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Truncates the file.
 * @param {string | Buffer | URL} path
 * @param {number} [len]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function truncate(path, len, callback) {
  if (typeof path === 'number') {
    showTruncateDeprecation();
    return fs.ftruncate(path, len, callback);
  }

  if (typeof len === 'function') {
    callback = len;
    len = 0;
  } else if (len === undefined) {
    len = 0;
  }

  validateInteger(len, 'len');
  len = MathMax(0, len);
  callback = maybeCallback(callback);
  fs.open(path, 'r+', function (er, fd) {
    if (er) return callback(er);
    var req = new FSReqCallback();

    req.oncomplete = function oncomplete(er) {
      fs.close(fd, function (er2) {
        callback(aggregateTwoErrors(er2, er));
      });
    };

    binding.ftruncate(fd, len, req);
  });
}
/**
 * Synchronously truncates the file.
 * @param {string | Buffer | URL} path
 * @param {number} [len]
 * @returns {void}
 */


function truncateSync(path, len) {
  if (typeof path === 'number') {
    // legacy
    showTruncateDeprecation();
    return fs.ftruncateSync(path, len);
  }

  if (len === undefined) {
    len = 0;
  } // Allow error to be thrown, but still close fd.


  var fd = fs.openSync(path, 'r+');
  var ret;

  try {
    ret = fs.ftruncateSync(fd, len);
  } finally {
    fs.closeSync(fd);
  }

  return ret;
}
/**
 * Truncates the file descriptor.
 * @param {number} fd
 * @param {number} [len]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function ftruncate(fd) {
  var len = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  var callback = arguments.length > 2 ? arguments[2] : undefined;

  if (typeof len === 'function') {
    callback = len;
    len = 0;
  }

  fd = getValidatedFd(fd);
  validateInteger(len, 'len');
  len = MathMax(0, len);
  callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.ftruncate(fd, len, req);
}
/**
 * Synchronously truncates the file descriptor.
 * @param {number} fd
 * @param {number} [len]
 * @returns {void}
 */


function ftruncateSync(fd) {
  var len = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;
  fd = getValidatedFd(fd);
  validateInteger(len, 'len');
  len = MathMax(0, len);
  var ctx = {};
  binding.ftruncate(fd, len, undefined, ctx);
  handleErrorFromBinding(ctx);
}

function lazyLoadRimraf() {
  if (rimraf === undefined) {
    var _require9 = require('internal/fs/rimraf');

    rimraf = _require9.rimraf;
    rimrafSync = _require9.rimrafSync;
  }
}
/**
 * Asynchronously removes a directory.
 * @param {string | Buffer | URL} path
 * @param {{
 *   maxRetries?: number;
 *   recursive?: boolean;
 *   retryDelay?: number;
 *   }} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function rmdir(path, options, callback) {
  var _options2;

  if (typeof options === 'function') {
    callback = options;
    options = undefined;
  }

  callback = makeCallback(callback);
  path = pathModule.toNamespacedPath(getValidatedPath(path));

  if ((_options2 = options) !== null && _options2 !== void 0 && _options2.recursive) {
    emitRecursiveRmdirWarning();
    validateRmOptions(path, _objectSpread(_objectSpread({}, options), {}, {
      force: false
    }), true, function (err, options) {
      if (err === false) {
        var req = new FSReqCallback();
        req.oncomplete = callback;
        return binding.rmdir(path, req);
      }

      if (err) {
        return callback(err);
      }

      lazyLoadRimraf();
      rimraf(path, options, callback);
    });
  } else {
    validateRmdirOptions(options);
    var req = new FSReqCallback();
    req.oncomplete = callback;
    return binding.rmdir(path, req);
  }
}
/**
 * Synchronously removes a directory.
 * @param {string | Buffer | URL} path
 * @param {{
 *   maxRetries?: number;
 *   recursive?: boolean;
 *   retryDelay?: number;
 *   }} [options]
 * @returns {void}
 */


function rmdirSync(path, options) {
  var _options3;

  path = getValidatedPath(path);

  if ((_options3 = options) !== null && _options3 !== void 0 && _options3.recursive) {
    emitRecursiveRmdirWarning();
    options = validateRmOptionsSync(path, _objectSpread(_objectSpread({}, options), {}, {
      force: false
    }), true);

    if (options !== false) {
      lazyLoadRimraf();
      return rimrafSync(pathModule.toNamespacedPath(path), options);
    }
  } else {
    validateRmdirOptions(options);
  }

  var ctx = {
    path: path
  };
  binding.rmdir(pathModule.toNamespacedPath(path), undefined, ctx);
  return handleErrorFromBinding(ctx);
}
/**
 * Asynchronously removes files and
 * directories (modeled on the standard POSIX `rm` utility).
 * @param {string | Buffer | URL} path
 * @param {{
 *   force?: boolean;
 *   maxRetries?: number;
 *   recursive?: boolean;
 *   retryDelay?: number;
 *   }} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function rm(path, options, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = undefined;
  }

  validateRmOptions(path, options, false, function (err, options) {
    if (err) {
      return callback(err);
    }

    lazyLoadRimraf();
    return rimraf(pathModule.toNamespacedPath(path), options, callback);
  });
}
/**
 * Synchronously removes files and
 * directories (modeled on the standard POSIX `rm` utility).
 * @param {string | Buffer | URL} path
 * @param {{
 *   force?: boolean;
 *   maxRetries?: number;
 *   recursive?: boolean;
 *   retryDelay?: number;
 *   }} [options]
 * @returns {void}
 */


function rmSync(path, options) {
  options = validateRmOptionsSync(path, options, false);
  lazyLoadRimraf();
  return rimrafSync(pathModule.toNamespacedPath(path), options);
}
/**
 * Forces all currently queued I/O operations associated
 * with the file to the operating system's synchronized
 * I/O completion state.
 * @param {number} fd
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function fdatasync(fd, callback) {
  fd = getValidatedFd(fd);
  var req = new FSReqCallback();
  req.oncomplete = makeCallback(callback);
  binding.fdatasync(fd, req);
}
/**
 * Synchronously forces all currently queued I/O operations
 * associated with the file to the operating
 * system's synchronized I/O completion state.
 * @param {number} fd
 * @returns {void}
 */


function fdatasyncSync(fd) {
  fd = getValidatedFd(fd);
  var ctx = {};
  binding.fdatasync(fd, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Requests for all data for the open file descriptor
 * to be flushed to the storage device.
 * @param {number} fd
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function fsync(fd, callback) {
  fd = getValidatedFd(fd);
  var req = new FSReqCallback();
  req.oncomplete = makeCallback(callback);
  binding.fsync(fd, req);
}
/**
 * Synchronously requests for all data for the open
 * file descriptor to be flushed to the storage device.
 * @param {number} fd
 * @returns {void}
 */


function fsyncSync(fd) {
  fd = getValidatedFd(fd);
  var ctx = {};
  binding.fsync(fd, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Asynchronously creates a directory.
 * @param {string | Buffer | URL} path
 * @param {{
 *   recursive?: boolean;
 *   mode?: string | number;
 *   } | number} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function mkdir(path, options, callback) {
  var mode = 511;
  var recursive = false;

  if (typeof options === 'function') {
    callback = options;
  } else if (typeof options === 'number' || typeof options === 'string') {
    mode = options;
  } else if (options) {
    if (options.recursive !== undefined) recursive = options.recursive;
    if (options.mode !== undefined) mode = options.mode;
  }

  callback = makeCallback(callback);
  path = getValidatedPath(path);
  validateBoolean(recursive, 'options.recursive');
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.mkdir(pathModule.toNamespacedPath(path), parseFileMode(mode, 'mode'), recursive, req);
}
/**
 * Synchronously creates a directory.
 * @param {string | Buffer | URL} path
 * @param {{
 *   recursive?: boolean;
 *   mode?: string | number;
 *   } | number} [options]
 * @returns {string | void}
 */


function mkdirSync(path, options) {
  var mode = 511;
  var recursive = false;

  if (typeof options === 'number' || typeof options === 'string') {
    mode = options;
  } else if (options) {
    if (options.recursive !== undefined) recursive = options.recursive;
    if (options.mode !== undefined) mode = options.mode;
  }

  path = getValidatedPath(path);
  validateBoolean(recursive, 'options.recursive');
  var ctx = {
    path: path
  };
  var result = binding.mkdir(pathModule.toNamespacedPath(path), parseFileMode(mode, 'mode'), recursive, undefined, ctx);
  handleErrorFromBinding(ctx);

  if (recursive) {
    return result;
  }
}
/**
 * Reads the contents of a directory.
 * @param {string | Buffer | URL} path
 * @param {string | {
 *   encoding?: string;
 *   withFileTypes?: boolean;
 *   }} [options]
 * @param {(
 *   err?: Error,
 *   files?: string[] | Buffer[] | Direct[];
 *   ) => any} callback
 * @returns {void}
 */


function readdir(path, options, callback) {
  callback = makeCallback(typeof options === 'function' ? options : callback);
  options = getOptions(options, {});
  path = getValidatedPath(path);
  var req = new FSReqCallback();

  if (!options.withFileTypes) {
    req.oncomplete = callback;
  } else {
    req.oncomplete = function (err, result) {
      if (err) {
        callback(err);
        return;
      }

      getDirents(path, result, callback);
    };
  }

  binding.readdir(pathModule.toNamespacedPath(path), options.encoding, !!options.withFileTypes, req);
}
/**
 * Synchronously reads the contents of a directory.
 * @param {string | Buffer | URL} path
 * @param {string | {
 *   encoding?: string;
 *   withFileTypes?: boolean;
 *   }} [options]
 * @returns {string | Buffer[] | Dirent[]}
 */


function readdirSync(path, options) {
  options = getOptions(options, {});
  path = getValidatedPath(path);
  var ctx = {
    path: path
  };
  var result = binding.readdir(pathModule.toNamespacedPath(path), options.encoding, !!options.withFileTypes, undefined, ctx);
  handleErrorFromBinding(ctx);
  return options.withFileTypes ? getDirents(path, result) : result;
}
/**
 * Invokes the callback with the `fs.Stats`
 * for the file descriptor.
 * @param {number} fd
 * @param {{ bigint?: boolean; }} [options]
 * @param {(
 *   err?: Error,
 *   stats?: Stats
 *   ) => any} callback
 * @returns {void}
 */


function fstat(fd) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {
    bigint: false
  };
  var callback = arguments.length > 2 ? arguments[2] : undefined;

  if (typeof options === 'function') {
    callback = options;
    options = {};
  }

  fd = getValidatedFd(fd);
  callback = makeStatsCallback(callback);
  var req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.fstat(fd, options.bigint, req);
}
/**
 * Retrieves the `fs.Stats` for the symbolic link
 * referred to by the `path`.
 * @param {string | Buffer | URL} path
 * @param {{ bigint?: boolean; }} [options]
 * @param {(
 *   err?: Error,
 *   stats?: Stats
 *   ) => any} callback
 * @returns {void}
 */


function lstat(path) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {
    bigint: false
  };
  var callback = arguments.length > 2 ? arguments[2] : undefined;

  if (typeof options === 'function') {
    callback = options;
    options = {};
  }

  callback = makeStatsCallback(callback);
  path = getValidatedPath(path);
  var req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.lstat(pathModule.toNamespacedPath(path), options.bigint, req);
}
/**
 * Asynchronously gets the stats of a file.
 * @param {string | Buffer | URL} path
 * @param {{ bigint?: boolean; }} [options]
 * @param {(
 *   err?: Error,
 *   stats?: Stats
 *   ) => any} callback
 * @returns {void}
 */


function stat(path) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {
    bigint: false
  };
  var callback = arguments.length > 2 ? arguments[2] : undefined;

  if (typeof options === 'function') {
    callback = options;
    options = {};
  }

  callback = makeStatsCallback(callback);
  path = getValidatedPath(path);
  var req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.stat(pathModule.toNamespacedPath(path), options.bigint, req);
}

function hasNoEntryError(ctx) {
  if (ctx.errno) {
    var uvErr = uvErrmapGet(ctx.errno);
    return (uvErr === null || uvErr === void 0 ? void 0 : uvErr[0]) === 'ENOENT';
  }

  if (ctx.error) {
    return ctx.error.code === 'ENOENT';
  }

  return false;
}
/**
 * Synchronously retrieves the `fs.Stats` for
 * the file descriptor.
 * @param {number} fd
 * @param {{
 *   bigint?: boolean;
 *   throwIfNoEntry?: boolean;
 *   }} [options]
 * @returns {Stats}
 */


function fstatSync(fd) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {
    bigint: false,
    throwIfNoEntry: true
  };
  fd = getValidatedFd(fd);
  var ctx = {
    fd: fd
  };
  var stats = binding.fstat(fd, options.bigint, undefined, ctx);
  handleErrorFromBinding(ctx);
  return getStatsFromBinding(stats);
}
/**
 * Synchronously retrieves the `fs.Stats` for
 * the symbolic link referred to by the `path`.
 * @param {string | Buffer | URL} path
 * @param {{
 *   bigint?: boolean;
 *   throwIfNoEntry?: boolean;
 *   }} [options]
 * @returns {Stats}
 */


function lstatSync(path) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {
    bigint: false,
    throwIfNoEntry: true
  };
  path = getValidatedPath(path);
  var ctx = {
    path: path
  };
  var stats = binding.lstat(pathModule.toNamespacedPath(path), options.bigint, undefined, ctx);

  if (options.throwIfNoEntry === false && hasNoEntryError(ctx)) {
    return undefined;
  }

  handleErrorFromBinding(ctx);
  return getStatsFromBinding(stats);
}
/**
 * Synchronously retrieves the `fs.Stats`
 * for the `path`.
 * @param {string | Buffer | URL} path
 * @param {{
 *   bigint?: boolean;
 *   throwIfNoEntry?: boolean;
 *   }} [options]
 * @returns {Stats}
 */


function statSync(path) {
  var options = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : {
    bigint: false,
    throwIfNoEntry: true
  };
  path = getValidatedPath(path);
  var ctx = {
    path: path
  };
  var stats = binding.stat(pathModule.toNamespacedPath(path), options.bigint, undefined, ctx);

  if (options.throwIfNoEntry === false && hasNoEntryError(ctx)) {
    return undefined;
  }

  handleErrorFromBinding(ctx);
  return getStatsFromBinding(stats);
}
/**
 * Reads the contents of a symbolic link
 * referred to by `path`.
 * @param {string | Buffer | URL} path
 * @param {{ encoding?: string; } | string} [options]
 * @param {(
 *   err?: Error,
 *   linkString?: string | Buffer
 *   ) => any} callback
 * @returns {void}
 */


function readlink(path, options, callback) {
  callback = makeCallback(typeof options === 'function' ? options : callback);
  options = getOptions(options, {});
  path = getValidatedPath(path, 'oldPath');
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.readlink(pathModule.toNamespacedPath(path), options.encoding, req);
}
/**
 * Synchronously reads the contents of a symbolic link
 * referred to by `path`.
 * @param {string | Buffer | URL} path
 * @param {{ encoding?: string; } | string} [options]
 * @returns {string | Buffer}
 */


function readlinkSync(path, options) {
  options = getOptions(options, {});
  path = getValidatedPath(path, 'oldPath');
  var ctx = {
    path: path
  };
  var result = binding.readlink(pathModule.toNamespacedPath(path), options.encoding, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}
/**
 * Creates the link called `path` pointing to `target`.
 * @param {string | Buffer | URL} target
 * @param {string | Buffer | URL} path
 * @param {string} [type_]
 * @param {(err?: Error) => any} callback_
 * @returns {void}
 */


function symlink(target, path, type_, callback_) {
  var type = typeof type_ === 'string' ? type_ : null;
  var callback = makeCallback(arguments[arguments.length - 1]);
  target = getValidatedPath(target, 'target');
  path = getValidatedPath(path);

  if (isWindows && type === null) {
    var absoluteTarget;

    try {
      // Symlinks targets can be relative to the newly created path.
      // Calculate absolute file name of the symlink target, and check
      // if it is a directory. Ignore resolve error to keep symlink
      // errors consistent between platforms if invalid path is
      // provided.
      absoluteTarget = pathModule.resolve(path, '..', target);
    } catch (_unused3) {}

    if (absoluteTarget !== undefined) {
      stat(absoluteTarget, function (err, stat) {
        var resolvedType = !err && stat.isDirectory() ? 'dir' : 'file';
        var resolvedFlags = stringToSymlinkType(resolvedType);
        var destination = preprocessSymlinkDestination(target, resolvedType, path);
        var req = new FSReqCallback();
        req.oncomplete = callback;
        binding.symlink(destination, pathModule.toNamespacedPath(path), resolvedFlags, req);
      });
      return;
    }
  }

  var destination = preprocessSymlinkDestination(target, type, path);
  var flags = stringToSymlinkType(type);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.symlink(destination, pathModule.toNamespacedPath(path), flags, req);
}
/**
 * Synchronously creates the link called `path`
 * pointing to `target`.
 * @param {string | Buffer | URL} target
 * @param {string | Buffer | URL} path
 * @param {string} [type]
 * @returns {void}
 */


function symlinkSync(target, path, type) {
  type = typeof type === 'string' ? type : null;

  if (isWindows && type === null) {
    var _statSync;

    var absoluteTarget = pathModule.resolve("".concat(path), '..', "".concat(target));

    if ((_statSync = statSync(absoluteTarget, {
      throwIfNoEntry: false
    })) !== null && _statSync !== void 0 && _statSync.isDirectory()) {
      type = 'dir';
    }
  }

  target = getValidatedPath(target, 'target');
  path = getValidatedPath(path);
  var flags = stringToSymlinkType(type);
  var ctx = {
    path: target,
    dest: path
  };
  binding.symlink(preprocessSymlinkDestination(target, type, path), pathModule.toNamespacedPath(path), flags, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Creates a new link from the `existingPath`
 * to the `newPath`.
 * @param {string | Buffer | URL} existingPath
 * @param {string | Buffer | URL} newPath
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function link(existingPath, newPath, callback) {
  callback = makeCallback(callback);
  existingPath = getValidatedPath(existingPath, 'existingPath');
  newPath = getValidatedPath(newPath, 'newPath');
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.link(pathModule.toNamespacedPath(existingPath), pathModule.toNamespacedPath(newPath), req);
}
/**
 * Synchronously creates a new link from the `existingPath`
 * to the `newPath`.
 * @param {string | Buffer | URL} existingPath
 * @param {string | Buffer | URL} newPath
 * @returns {void}
 */


function linkSync(existingPath, newPath) {
  existingPath = getValidatedPath(existingPath, 'existingPath');
  newPath = getValidatedPath(newPath, 'newPath');
  var ctx = {
    path: existingPath,
    dest: newPath
  };
  var result = binding.link(pathModule.toNamespacedPath(existingPath), pathModule.toNamespacedPath(newPath), undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}
/**
 * Asynchronously removes a file or symbolic link.
 * @param {string | Buffer | URL} path
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function unlink(path, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.unlink(pathModule.toNamespacedPath(path), req);
}
/**
 * Synchronously removes a file or symbolic link.
 * @param {string | Buffer | URL} path
 * @returns {void}
 */


function unlinkSync(path) {
  path = getValidatedPath(path);
  var ctx = {
    path: path
  };
  binding.unlink(pathModule.toNamespacedPath(path), undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Sets the permissions on the file.
 * @param {number} fd
 * @param {string | number} mode
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function fchmod(fd, mode, callback) {
  fd = getValidatedFd(fd);
  mode = parseFileMode(mode, 'mode');
  callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchmod(fd, mode, req);
}
/**
 * Synchronously sets the permissions on the file.
 * @param {number} fd
 * @param {string | number} mode
 * @returns {void}
 */


function fchmodSync(fd, mode) {
  fd = getValidatedFd(fd);
  mode = parseFileMode(mode, 'mode');
  var ctx = {};
  binding.fchmod(fd, mode, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Changes the permissions on a symbolic link.
 * @param {string | Buffer | URL} path
 * @param {number} mode
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function lchmod(path, mode, callback) {
  callback = maybeCallback(callback);
  mode = parseFileMode(mode, 'mode');
  fs.open(path, O_WRONLY | O_SYMLINK, function (err, fd) {
    if (err) {
      callback(err);
      return;
    } // Prefer to return the chmod error, if one occurs,
    // but still try to close, and report closing errors if they occur.


    fs.fchmod(fd, mode, function (err) {
      fs.close(fd, function (err2) {
        callback(aggregateTwoErrors(err2, err));
      });
    });
  });
}
/**
 * Synchronously changes the permissions on a symbolic link.
 * @param {string | Buffer | URL} path
 * @param {number} mode
 * @returns {void}
 */


function lchmodSync(path, mode) {
  var fd = fs.openSync(path, O_WRONLY | O_SYMLINK); // Prefer to return the chmod error, if one occurs,
  // but still try to close, and report closing errors if they occur.

  var ret;

  try {
    ret = fs.fchmodSync(fd, mode);
  } finally {
    fs.closeSync(fd);
  }

  return ret;
}
/**
 * Asynchronously changes the permissions of a file.
 * @param {string | Buffer | URL} path
 * @param {string | number} mode
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function chmod(path, mode, callback) {
  path = getValidatedPath(path);
  mode = parseFileMode(mode, 'mode');
  callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.chmod(pathModule.toNamespacedPath(path), mode, req);
}
/**
 * Synchronously changes the permissions of a file.
 * @param {string | Buffer | URL} path
 * @param {string | number} mode
 * @returns {void}
 */


function chmodSync(path, mode) {
  path = getValidatedPath(path);
  mode = parseFileMode(mode, 'mode');
  var ctx = {
    path: path
  };
  binding.chmod(pathModule.toNamespacedPath(path), mode, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Sets the owner of the symbolic link.
 * @param {string | Buffer | URL} path
 * @param {number} uid
 * @param {number} gid
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function lchown(path, uid, gid, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.lchown(pathModule.toNamespacedPath(path), uid, gid, req);
}
/**
 * Synchronously sets the owner of the symbolic link.
 * @param {string | Buffer | URL} path
 * @param {number} uid
 * @param {number} gid
 * @returns {void}
 */


function lchownSync(path, uid, gid) {
  path = getValidatedPath(path);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  var ctx = {
    path: path
  };
  binding.lchown(pathModule.toNamespacedPath(path), uid, gid, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Sets the owner of the file.
 * @param {number} fd
 * @param {number} uid
 * @param {number} gid
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function fchown(fd, uid, gid, callback) {
  fd = getValidatedFd(fd);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchown(fd, uid, gid, req);
}
/**
 * Synchronously sets the owner of the file.
 * @param {number} fd
 * @param {number} uid
 * @param {number} gid
 * @returns {void}
 */


function fchownSync(fd, uid, gid) {
  fd = getValidatedFd(fd);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  var ctx = {};
  binding.fchown(fd, uid, gid, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Asynchronously changes the owner and group
 * of a file.
 * @param {string | Buffer | URL} path
 * @param {number} uid
 * @param {number} gid
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function chown(path, uid, gid, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.chown(pathModule.toNamespacedPath(path), uid, gid, req);
}
/**
 * Synchronously changes the owner and group
 * of a file.
 * @param {string | Buffer | URL} path
 * @param {number} uid
 * @param {number} gid
 * @returns {void}
 */


function chownSync(path, uid, gid) {
  path = getValidatedPath(path);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  var ctx = {
    path: path
  };
  binding.chown(pathModule.toNamespacedPath(path), uid, gid, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Changes the file system timestamps of the object
 * referenced by `path`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function utimes(path, atime, mtime, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.utimes(pathModule.toNamespacedPath(path), toUnixTimestamp(atime), toUnixTimestamp(mtime), req);
}
/**
 * Synchronously changes the file system timestamps
 * of the object referenced by `path`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */


function utimesSync(path, atime, mtime) {
  path = getValidatedPath(path);
  var ctx = {
    path: path
  };
  binding.utimes(pathModule.toNamespacedPath(path), toUnixTimestamp(atime), toUnixTimestamp(mtime), undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Changes the file system timestamps of the object
 * referenced by the supplied `fd` (file descriptor).
 * @param {number} fd
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function futimes(fd, atime, mtime, callback) {
  fd = getValidatedFd(fd);
  atime = toUnixTimestamp(atime, 'atime');
  mtime = toUnixTimestamp(mtime, 'mtime');
  callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.futimes(fd, atime, mtime, req);
}
/**
 * Synchronously changes the file system timestamps
 * of the object referenced by the
 * supplied `fd` (file descriptor).
 * @param {number} fd
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */


function futimesSync(fd, atime, mtime) {
  fd = getValidatedFd(fd);
  atime = toUnixTimestamp(atime, 'atime');
  mtime = toUnixTimestamp(mtime, 'mtime');
  var ctx = {};
  binding.futimes(fd, atime, mtime, undefined, ctx);
  handleErrorFromBinding(ctx);
}
/**
 * Changes the access and modification times of
 * a file in the same way as `fs.utimes()`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function lutimes(path, atime, mtime, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.lutimes(pathModule.toNamespacedPath(path), toUnixTimestamp(atime), toUnixTimestamp(mtime), req);
}
/**
 * Synchronously changes the access and modification
 * times of a file in the same way as `fs.utimesSync()`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */


function lutimesSync(path, atime, mtime) {
  path = getValidatedPath(path);
  var ctx = {
    path: path
  };
  binding.lutimes(pathModule.toNamespacedPath(path), toUnixTimestamp(atime), toUnixTimestamp(mtime), undefined, ctx);
  handleErrorFromBinding(ctx);
}

function writeAll(fd, isUserFd, buffer, offset, length, signal, callback) {
  if (signal !== null && signal !== void 0 && signal.aborted) {
    var abortError = new AbortError();

    if (isUserFd) {
      callback(abortError);
    } else {
      fs.close(fd, function (err) {
        callback(aggregateTwoErrors(err, abortError));
      });
    }

    return;
  } // write(fd, buffer, offset, length, position, callback)


  fs.write(fd, buffer, offset, length, null, function (writeErr, written) {
    if (writeErr) {
      if (isUserFd) {
        callback(writeErr);
      } else {
        fs.close(fd, function (err) {
          callback(aggregateTwoErrors(err, writeErr));
        });
      }
    } else if (written === length) {
      if (isUserFd) {
        callback(null);
      } else {
        fs.close(fd, callback);
      }
    } else {
      offset += written;
      length -= written;
      writeAll(fd, isUserFd, buffer, offset, length, signal, callback);
    }
  });
}
/**
 * Asynchronously writes data to the file.
 * @param {string | Buffer | URL | number} path
 * @param {string | Buffer | TypedArray | DataView | Object} data
 * @param {{
 *   encoding?: string | null;
 *   mode?: number;
 *   flag?: string;
 *   signal?: AbortSignal;
 *   } | string} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function writeFile(path, data, options, callback) {
  callback = maybeCallback(callback || options);
  options = getOptions(options, {
    encoding: 'utf8',
    mode: 438,
    flag: 'w'
  });
  var flag = options.flag || 'w';

  if (!isArrayBufferView(data)) {
    validateStringAfterArrayBufferView(data, 'data');
    data = Buffer.from(String(data), options.encoding || 'utf8');
  }

  if (isFd(path)) {
    var isUserFd = true;
    var signal = options.signal;
    writeAll(path, isUserFd, data, 0, data.byteLength, signal, callback);
    return;
  }

  if (checkAborted(options.signal, callback)) return;
  fs.open(path, flag, options.mode, function (openErr, fd) {
    if (openErr) {
      callback(openErr);
    } else {
      var _isUserFd = false;
      var _signal = options.signal;
      writeAll(fd, _isUserFd, data, 0, data.byteLength, _signal, callback);
    }
  });
}
/**
 * Synchronously writes data to the file.
 * @param {string | Buffer | URL | number} path
 * @param {string | Buffer | TypedArray | DataView | Object} data
 * @param {{
 *   encoding?: string | null;
 *   mode?: number;
 *   flag?: string;
 *   } | string} [options]
 * @returns {void}
 */


function writeFileSync(path, data, options) {
  options = getOptions(options, {
    encoding: 'utf8',
    mode: 438,
    flag: 'w'
  });

  if (!isArrayBufferView(data)) {
    validateStringAfterArrayBufferView(data, 'data');
    data = Buffer.from(String(data), options.encoding || 'utf8');
  }

  var flag = options.flag || 'w';
  var isUserFd = isFd(path); // File descriptor ownership

  var fd = isUserFd ? path : fs.openSync(path, flag, options.mode);
  var offset = 0;
  var length = data.byteLength;

  try {
    while (length > 0) {
      var written = fs.writeSync(fd, data, offset, length);
      offset += written;
      length -= written;
    }
  } finally {
    if (!isUserFd) fs.closeSync(fd);
  }
}
/**
 * Asynchronously appends data to a file.
 * @param {string | Buffer | URL | number} path
 * @param {string | Buffer} data
 * @param {{
 *   encoding?: string | null;
 *   mode?: number;
 *   flag?: string;
 *   } | string} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */


function appendFile(path, data, options, callback) {
  callback = maybeCallback(callback || options);
  options = getOptions(options, {
    encoding: 'utf8',
    mode: 438,
    flag: 'a'
  }); // Don't make changes directly on options object

  options = copyObject(options); // Force append behavior when using a supplied file descriptor

  if (!options.flag || isFd(path)) options.flag = 'a';
  fs.writeFile(path, data, options, callback);
}
/**
 * Synchronously appends data to a file.
 * @param {string | Buffer | URL | number} path
 * @param {string | Buffer} data
 * @param {{
 *   encoding?: string | null;
 *   mode?: number;
 *   flag?: string;
 *   } | string} [options]
 * @returns {void}
 */


function appendFileSync(path, data, options) {
  options = getOptions(options, {
    encoding: 'utf8',
    mode: 438,
    flag: 'a'
  }); // Don't make changes directly on options object

  options = copyObject(options); // Force append behavior when using a supplied file descriptor

  if (!options.flag || isFd(path)) options.flag = 'a';
  fs.writeFileSync(path, data, options);
}
/**
 * Watches for the changes on `filename`.
 * @param {string | Buffer | URL} filename
 * @param {string | {
 *   persistent?: boolean;
 *   recursive?: boolean;
 *   encoding?: string;
 *   signal?: AbortSignal;
 *   }} [options]
 * @param {(
 *   eventType?: string,
 *   filename?: string | Buffer
 *   ) => any} [listener]
 * @returns {watchers.FSWatcher}
 */


function watch(filename, options, listener) {
  if (typeof options === 'function') {
    listener = options;
  }

  options = getOptions(options, {}); // Don't make changes directly on options object

  options = copyObject(options);
  if (options.persistent === undefined) options.persistent = true;
  if (options.recursive === undefined) options.recursive = false;
  if (options.recursive && !(isOSX || isWindows)) throw new ERR_FEATURE_UNAVAILABLE_ON_PLATFORM('watch recursively');
  var watcher = new watchers.FSWatcher();
  watcher[watchers.kFSWatchStart](filename, options.persistent, options.recursive, options.encoding);

  if (listener) {
    watcher.addListener('change', listener);
  }

  if (options.signal) {
    if (options.signal.aborted) {
      process.nextTick(function () {
        return watcher.close();
      });
    } else {
      var _listener = function _listener() {
        return watcher.close();
      };

      options.signal.addEventListener('abort', _listener);
      watcher.once('close', function () {
        options.signal.removeEventListener('abort', _listener);
      });
    }
  }

  return watcher;
}

var statWatchers = new SafeMap();
/**
 * Watches for changes on `filename`.
 * @param {string | Buffer | URL} filename
 * @param {{
 *   bigint?: boolean;
 *   persistent?: boolean;
 *   interval?: number;
 *   }} [options]
 * @param {(
 *   current?: Stats,
 *   previous?: Stats
 *   ) => any} listener
 * @returns {watchers.StatWatcher}
 */

function watchFile(filename, options, listener) {
  filename = getValidatedPath(filename);
  filename = pathModule.resolve(filename);
  var stat;

  if (options === null || _typeof(options) !== 'object') {
    listener = options;
    options = null;
  }

  options = _objectSpread({
    // Poll interval in milliseconds. 5007 is what libev used to use. It's
    // a little on the slow side but let's stick with it for now to keep
    // behavioral changes to a minimum.
    interval: 5007,
    persistent: true
  }, options);
  validateFunction(listener, 'listener');
  stat = statWatchers.get(filename);

  if (stat === undefined) {
    stat = new watchers.StatWatcher(options.bigint);
    stat[watchers.kFSStatWatcherStart](filename, options.persistent, options.interval);
    statWatchers.set(filename, stat);
  } else {
    stat[watchers.kFSStatWatcherAddOrCleanRef]('add');
  }

  stat.addListener('change', listener);
  return stat;
}
/**
 * Stops watching for changes on `filename`.
 * @param {string | Buffer | URL} filename
 * @param {() => any} [listener]
 * @returns {void}
 */


function unwatchFile(filename, listener) {
  filename = getValidatedPath(filename);
  filename = pathModule.resolve(filename);
  var stat = statWatchers.get(filename);
  if (stat === undefined) return;

  if (typeof listener === 'function') {
    var beforeListenerCount = stat.listenerCount('change');
    stat.removeListener('change', listener);
    if (stat.listenerCount('change') < beforeListenerCount) stat[watchers.kFSStatWatcherAddOrCleanRef]('clean');
  } else {
    stat.removeAllListeners('change');
    stat[watchers.kFSStatWatcherAddOrCleanRef]('cleanAll');
  }

  if (stat.listenerCount('change') === 0) {
    stat.stop();
    statWatchers["delete"](filename);
  }
}

var splitRoot;

// if (isWindows) {
//   // Regex to find the device root on Windows (e.g. 'c:\\'), including trailing
//   // slash.
//   var splitRootRe = /^(?:[a-zA-Z]:|[\\/]{2}[^\\/]+[\\/][^\\/]+)?[\\/]*/;

//   splitRoot = function splitRoot(str) {
//     return RegExpPrototypeExec(splitRootRe, str)[0];
//   };
// } else {
//   splitRoot = function splitRoot(str) {
//     for (var i = 0; i < str.length; ++i) {
//       if (StringPrototypeCharCodeAt(str, i) !== CHAR_FORWARD_SLASH) return StringPrototypeSlice(str, 0, i);
//     }

//     return str;
//   };
// }

function encodeRealpathResult(result, options) {
  if (!options || !options.encoding || options.encoding === 'utf8') return result;
  var asBuffer = Buffer.from(result);

  if (options.encoding === 'buffer') {
    return asBuffer;
  }

  return asBuffer.toString(options.encoding);
} // Finds the next portion of a (partial) path, up to the next path delimiter


var nextPart;

// if (isWindows) {
//   nextPart = function nextPart(p, i) {
//     for (; i < p.length; ++i) {
//       var ch = StringPrototypeCharCodeAt(p, i); // Check for a separator character

//       if (ch === CHAR_BACKWARD_SLASH || ch === CHAR_FORWARD_SLASH) return i;
//     }

//     return -1;
//   };
// } else {
//   nextPart = function nextPart(p, i) {
//     return StringPrototypeIndexOf(p, '/', i);
//   };
// }

var emptyObj = ObjectCreate(null);
/**
 * Returns the resolved pathname.
 * @param {string | Buffer | URL} p
 * @param {string | { encoding?: string | null; }} [options]
 * @returns {string | Buffer}
 */

function realpathSync(p, options) {
  options = getOptions(options, emptyObj);
  p = toPathIfFileURL(p);

  if (typeof p !== 'string') {
    p += '';
  }

  validatePath(p);
  p = pathModule.resolve(p);
  var cache = options[realpathCacheKey];
  var maybeCachedResult = cache === null || cache === void 0 ? void 0 : cache.get(p);

  if (maybeCachedResult) {
    return maybeCachedResult;
  }

  var seenLinks = ObjectCreate(null);
  var knownHard = ObjectCreate(null);
  var original = p; // Current character position in p

  var pos; // The partial path so far, including a trailing slash if any

  var current; // The partial path without a trailing slash (except when pointing at a root)

  var base; // The partial path scanned in the previous round, with slash

  var previous; // Skip over roots

  current = base = splitRoot(p);
  pos = current.length; // On windows, check that the root exists. On unix there is no need.

  if (isWindows) {
    var ctx = {
      path: base
    };
    binding.lstat(pathModule.toNamespacedPath(base), false, undefined, ctx);
    handleErrorFromBinding(ctx);
    knownHard[base] = true;
  } // Walk down the path, swapping out linked path parts for their real
  // values
  // NB: p.length changes.


  while (pos < p.length) {
    // find the next part
    var result = nextPart(p, pos);
    previous = current;

    if (result === -1) {
      var last = StringPrototypeSlice(p, pos);
      current += last;
      base = previous + last;
      pos = p.length;
    } else {
      current += StringPrototypeSlice(p, pos, result + 1);
      base = previous + StringPrototypeSlice(p, pos, result);
      pos = result + 1;
    } // Continue if not a symlink, break if a pipe/socket


    if (knownHard[base] || (cache === null || cache === void 0 ? void 0 : cache.get(base)) === base) {
      if (isFileType(binding.statValues, S_IFIFO) || isFileType(binding.statValues, S_IFSOCK)) {
        break;
      }

      continue;
    }

    var resolvedLink = void 0;
    var maybeCachedResolved = cache === null || cache === void 0 ? void 0 : cache.get(base);

    if (maybeCachedResolved) {
      resolvedLink = maybeCachedResolved;
    } else {
      // Use stats array directly to avoid creating an fs.Stats instance just
      // for our internal use.
      var baseLong = pathModule.toNamespacedPath(base);
      var _ctx = {
        path: base
      };
      var stats = binding.lstat(baseLong, true, undefined, _ctx);
      handleErrorFromBinding(_ctx);

      if (!isFileType(stats, S_IFLNK)) {
        knownHard[base] = true;
        cache === null || cache === void 0 ? void 0 : cache.set(base, base);
        continue;
      } // Read the link if it wasn't read before
      // dev/ino always return 0 on windows, so skip the check.


      var linkTarget = null;
      var id = void 0;

      if (!isWindows) {
        var dev = BigIntPrototypeToString(stats[0], 32);
        var ino = BigIntPrototypeToString(stats[7], 32);
        id = "".concat(dev, ":").concat(ino);

        if (seenLinks[id]) {
          linkTarget = seenLinks[id];
        }
      }

      if (linkTarget === null) {
        var _ctx2 = {
          path: base
        };
        binding.stat(baseLong, false, undefined, _ctx2);
        handleErrorFromBinding(_ctx2);
        linkTarget = binding.readlink(baseLong, undefined, undefined, _ctx2);
        handleErrorFromBinding(_ctx2);
      }

      resolvedLink = pathModule.resolve(previous, linkTarget);
      if (cache) cache.set(base, resolvedLink);
      if (!isWindows) seenLinks[id] = linkTarget;
    } // Resolve the link, then start over


    p = pathModule.resolve(resolvedLink, StringPrototypeSlice(p, pos)); // Skip over roots

    current = base = splitRoot(p);
    pos = current.length; // On windows, check that the root exists. On unix there is no need.

    if (isWindows && !knownHard[base]) {
      var _ctx3 = {
        path: base
      };
      binding.lstat(pathModule.toNamespacedPath(base), false, undefined, _ctx3);
      handleErrorFromBinding(_ctx3);
      knownHard[base] = true;
    }
  }

  cache === null || cache === void 0 ? void 0 : cache.set(original, p);
  return encodeRealpathResult(p, options);
}
/**
 * Returns the resolved pathname.
 * @param {string | Buffer | URL} p
 * @param {string | { encoding?: string; }} [options]
 * @returns {string | Buffer}
 */


realpathSync["native"] = function (path, options) {
  options = getOptions(options, {});
  path = getValidatedPath(path);
  var ctx = {
    path: path
  };
  var result = binding.realpath(path, options.encoding, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
};
/**
 * Asynchronously computes the canonical pathname by
 * resolving `.`, `..` and symbolic links.
 * @param {string | Buffer | URL} p
 * @param {string | { encoding?: string; }} [options]
 * @param {(
 *   err?: Error,
 *   resolvedPath?: string | Buffer
 *   ) => any} callback
 * @returns {void}
 */


function realpath(p, options, callback) {
  callback = typeof options === 'function' ? options : maybeCallback(callback);
  options = getOptions(options, {});
  p = toPathIfFileURL(p);

  if (typeof p !== 'string') {
    p += '';
  }

  validatePath(p);
  p = pathModule.resolve(p);
  var seenLinks = ObjectCreate(null);
  var knownHard = ObjectCreate(null); // Current character position in p

  var pos; // The partial path so far, including a trailing slash if any

  var current; // The partial path without a trailing slash (except when pointing at a root)

  var base; // The partial path scanned in the previous round, with slash

  var previous;
  current = base = splitRoot(p);
  pos = current.length; // On windows, check that the root exists. On unix there is no need.

  if (isWindows && !knownHard[base]) {
    fs.lstat(base, function (err, stats) {
      if (err) return callback(err);
      knownHard[base] = true;
      LOOP();
    });
  } else {
    process.nextTick(LOOP);
  } // Walk down the path, swapping out linked path parts for their real
  // values


  function LOOP() {
    // Stop if scanned past end of path
    if (pos >= p.length) {
      return callback(null, encodeRealpathResult(p, options));
    } // find the next part


    var result = nextPart(p, pos);
    previous = current;

    if (result === -1) {
      var last = StringPrototypeSlice(p, pos);
      current += last;
      base = previous + last;
      pos = p.length;
    } else {
      current += StringPrototypeSlice(p, pos, result + 1);
      base = previous + StringPrototypeSlice(p, pos, result);
      pos = result + 1;
    } // Continue if not a symlink, break if a pipe/socket


    if (knownHard[base]) {
      if (isFileType(binding.statValues, S_IFIFO) || isFileType(binding.statValues, S_IFSOCK)) {
        return callback(null, encodeRealpathResult(p, options));
      }

      return process.nextTick(LOOP);
    }

    return fs.lstat(base, {
      bigint: true
    }, gotStat);
  }

  function gotStat(err, stats) {
    if (err) return callback(err); // If not a symlink, skip to the next path part

    if (!stats.isSymbolicLink()) {
      knownHard[base] = true;
      return process.nextTick(LOOP);
    } // Stat & read the link if not read before.
    // Call `gotTarget()` as soon as the link target is known.
    // `dev`/`ino` always return 0 on windows, so skip the check.


    var id;

    if (!isWindows) {
      var dev = BigIntPrototypeToString(stats.dev, 32);
      var ino = BigIntPrototypeToString(stats.ino, 32);
      id = "".concat(dev, ":").concat(ino);

      if (seenLinks[id]) {
        return gotTarget(null, seenLinks[id]);
      }
    }

    fs.stat(base, function (err) {
      if (err) return callback(err);
      fs.readlink(base, function (err, target) {
        if (!isWindows) seenLinks[id] = target;
        gotTarget(err, target);
      });
    });
  }

  function gotTarget(err, target) {
    if (err) return callback(err);
    gotResolvedLink(pathModule.resolve(previous, target));
  }

  function gotResolvedLink(resolvedLink) {
    // Resolve the link, then start over
    p = pathModule.resolve(resolvedLink, StringPrototypeSlice(p, pos));
    current = base = splitRoot(p);
    pos = current.length; // On windows, check that the root exists. On unix there is no need.

    if (isWindows && !knownHard[base]) {
      fs.lstat(base, function (err) {
        if (err) return callback(err);
        knownHard[base] = true;
        LOOP();
      });
    } else {
      process.nextTick(LOOP);
    }
  }
}
/**
 * Asynchronously computes the canonical pathname by
 * resolving `.`, `..` and symbolic links.
 * @param {string | Buffer | URL} p
 * @param {string | { encoding?: string; }} [options]
 * @param {(
 *   err?: Error,
 *   resolvedPath?: string | Buffer
 *   ) => any} callback
 * @returns {void}
 */


realpath["native"] = function (path, options, callback) {
  callback = makeCallback(callback || options);
  options = getOptions(options, {});
  path = getValidatedPath(path);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  return binding.realpath(path, options.encoding, req);
};
/**
 * Creates a unique temporary directory.
 * @param {string} prefix
 * @param {string | { encoding?: string; }} [options]
 * @param {(
 *   err?: Error,
 *   directory?: string
 *   ) => any} callback
 * @returns {void}
 */


function mkdtemp(prefix, options, callback) {
  callback = makeCallback(typeof options === 'function' ? options : callback);
  options = getOptions(options, {});

  if (!prefix || typeof prefix !== 'string') {
    throw new ERR_INVALID_ARG_TYPE('prefix', 'string', prefix);
  }

  nullCheck(prefix, 'prefix');
  warnOnNonPortableTemplate(prefix);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.mkdtemp("".concat(prefix, "XXXXXX"), options.encoding, req);
}
/**
 * Synchronously creates a unique temporary directory.
 * @param {string} prefix
 * @param {string | { encoding?: string; }} [options]
 * @returns {string}
 */


function mkdtempSync(prefix, options) {
  options = getOptions(options, {});

  if (!prefix || typeof prefix !== 'string') {
    throw new ERR_INVALID_ARG_TYPE('prefix', 'string', prefix);
  }

  nullCheck(prefix, 'prefix');
  warnOnNonPortableTemplate(prefix);
  var path = "".concat(prefix, "XXXXXX");
  var ctx = {
    path: path
  };
  var result = binding.mkdtemp(path, options.encoding, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}
/**
 * Asynchronously copies `src` to `dest`. By
 * default, `dest` is overwritten if it already exists.
 * @param {string | Buffer | URL} src
 * @param {string | Buffer | URL} dest
 * @param {number} [mode]
 * @param {() => any} callback
 * @returns {void}
 */


function copyFile(src, dest, mode, callback) {
  if (typeof mode === 'function') {
    callback = mode;
    mode = 0;
  }

  src = getValidatedPath(src, 'src');
  dest = getValidatedPath(dest, 'dest');
  src = pathModule._makeLong(src);
  dest = pathModule._makeLong(dest);
  mode = getValidMode(mode, 'copyFile');
  callback = makeCallback(callback);
  var req = new FSReqCallback();
  req.oncomplete = callback;
  binding.copyFile(src, dest, mode, req);
}
/**
 * Synchronously copies `src` to `dest`. By
 * default, `dest` is overwritten if it already exists.
 * @param {string | Buffer | URL} src
 * @param {string | Buffer | URL} dest
 * @param {number} [mode]
 * @returns {void}
 */


function copyFileSync(src, dest, mode) {
  src = getValidatedPath(src, 'src');
  dest = getValidatedPath(dest, 'dest');
  var ctx = {
    path: src,
    dest: dest
  }; // non-prefixed

  src = pathModule._makeLong(src);
  dest = pathModule._makeLong(dest);
  mode = getValidMode(mode, 'copyFile');
  binding.copyFile(src, dest, mode, undefined, ctx);
  handleErrorFromBinding(ctx);
}

function lazyLoadStreams() {
  if (!ReadStream) {
    var _require10 = require('internal/fs/streams');

    ReadStream = _require10.ReadStream;
    WriteStream = _require10.WriteStream;
    FileReadStream = ReadStream;
    FileWriteStream = WriteStream;
  }
}
/**
 * Creates a readable stream with a default `highWaterMark`
 * of 64 kb.
 * @param {string | Buffer | URL} path
 * @param {string | {
 *   flags?: string;
 *   encoding?: string;
 *   fd?: number | FileHandle;
 *   mode?: number;
 *   autoClose?: boolean;
 *   emitClose?: boolean;
 *   start: number;
 *   end?: number;
 *   highWaterMark?: number;
 *   fs?: Object | null;
 *   }} [options]
 * @returns {ReadStream}
 */


function createReadStream(path, options) {
  lazyLoadStreams();
  return new ReadStream(path, options);
}
/**
 * Creates a write stream.
 * @param {string | Buffer | URL} path
 * @param {string | {
 *   flags?: string;
 *   encoding?: string;
 *   fd?: number | FileHandle;
 *   mode?: number;
 *   autoClose?: boolean;
 *   emitClose?: boolean;
 *   start: number;
 *   fs?: Object | null;
 *   }} [options]
 * @returns {WriteStream}
 */


function createWriteStream(path, options) {
  lazyLoadStreams();
  return new WriteStream(path, options);
}

module.exports = fs = {
  appendFile: appendFile,
  appendFileSync: appendFileSync,
  access: access,
  accessSync: accessSync,
  chown: chown,
  chownSync: chownSync,
  chmod: chmod,
  chmodSync: chmodSync,
  close: close,
  closeSync: closeSync,
  copyFile: copyFile,
  copyFileSync: copyFileSync,
  createReadStream: createReadStream,
  createWriteStream: createWriteStream,
  exists: exists,
  existsSync: existsSync,
  fchown: fchown,
  fchownSync: fchownSync,
  fchmod: fchmod,
  fchmodSync: fchmodSync,
  fdatasync: fdatasync,
  fdatasyncSync: fdatasyncSync,
  fstat: fstat,
  fstatSync: fstatSync,
  fsync: fsync,
  fsyncSync: fsyncSync,
  ftruncate: ftruncate,
  ftruncateSync: ftruncateSync,
  futimes: futimes,
  futimesSync: futimesSync,
  lchown: lchown,
  lchownSync: lchownSync,
  lchmod: constants.O_SYMLINK !== undefined ? lchmod : undefined,
  lchmodSync: constants.O_SYMLINK !== undefined ? lchmodSync : undefined,
  link: link,
  linkSync: linkSync,
  lstat: lstat,
  lstatSync: lstatSync,
  lutimes: lutimes,
  lutimesSync: lutimesSync,
  mkdir: mkdir,
  mkdirSync: mkdirSync,
  mkdtemp: mkdtemp,
  mkdtempSync: mkdtempSync,
  open: open,
  openSync: openSync,
  // opendir: opendir,
  // opendirSync: opendirSync,
  readdir: readdir,
  readdirSync: readdirSync,
  read: read,
  readSync: readSync,
  readv: readv,
  readvSync: readvSync,
  readFile: readFile,
  readFileSync: readFileSync,
  readlink: readlink,
  readlinkSync: readlinkSync,
  realpath: realpath,
  realpathSync: realpathSync,
  rename: rename,
  renameSync: renameSync,
  rm: rm,
  rmSync: rmSync,
  rmdir: rmdir,
  rmdirSync: rmdirSync,
  stat: stat,
  statSync: statSync,
  symlink: symlink,
  symlinkSync: symlinkSync,
  truncate: truncate,
  truncateSync: truncateSync,
  unwatchFile: unwatchFile,
  unlink: unlink,
  unlinkSync: unlinkSync,
  utimes: utimes,
  utimesSync: utimesSync,
  watch: watch,
  watchFile: watchFile,
  writeFile: writeFile,
  writeFileSync: writeFileSync,
  write: write,
  writeSync: writeSync,
  writev: writev,
  writevSync: writevSync,
  // Dir: Dir,
  // Dirent: Dirent,
  // Stats: Stats,

  get ReadStream() {
    lazyLoadStreams();
    return ReadStream;
  },

  set ReadStream(val) {
    ReadStream = val;
  },

  get WriteStream() {
    lazyLoadStreams();
    return WriteStream;
  },

  set WriteStream(val) {
    WriteStream = val;
  },

  // Legacy names... these have to be separate because of how graceful-fs
  // (and possibly other) modules monkey patch the values.
  get FileReadStream() {
    lazyLoadStreams();
    return FileReadStream;
  },

  set FileReadStream(val) {
    FileReadStream = val;
  },

  get FileWriteStream() {
    lazyLoadStreams();
    return FileWriteStream;
  },

  set FileWriteStream(val) {
    FileWriteStream = val;
  },

  // For tests
  // _toUnixTimestamp: toUnixTimestamp
};
ObjectDefineProperties(fs, {
  F_OK: {
    enumerable: true,
    value: F_OK || 0
  },
  R_OK: {
    enumerable: true,
    value: R_OK || 0
  },
  W_OK: {
    enumerable: true,
    value: W_OK || 0
  },
  X_OK: {
    enumerable: true,
    value: X_OK || 0
  },
  constants: {
    configurable: false,
    enumerable: true,
    value: constants
  },
  promises: {
    configurable: true,
    enumerable: true,
    get: function get() {
      if (promises === null) promises = require('internal/fs/promises').exports;
      return promises;
    }
  }
});
