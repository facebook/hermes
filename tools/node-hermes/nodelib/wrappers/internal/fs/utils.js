// @nolint
'use strict';

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var _primordials = primordials,
    ArrayIsArray = _primordials.ArrayIsArray,
    BigInt = _primordials.BigInt,
    Date = _primordials.Date,
    DateNow = _primordials.DateNow,
    DatePrototypeGetTime = _primordials.DatePrototypeGetTime,
    ErrorCaptureStackTrace = _primordials.ErrorCaptureStackTrace,
    FunctionPrototypeCall = _primordials.FunctionPrototypeCall,
    Number = _primordials.Number,
    NumberIsFinite = _primordials.NumberIsFinite,
    NumberIsInteger = _primordials.NumberIsInteger,
    MathMin = _primordials.MathMin,
    ObjectIs = _primordials.ObjectIs,
    ObjectPrototypeHasOwnProperty = _primordials.ObjectPrototypeHasOwnProperty,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    ReflectApply = _primordials.ReflectApply,
    ReflectOwnKeys = _primordials.ReflectOwnKeys,
    StringPrototypeEndsWith = _primordials.StringPrototypeEndsWith,
    StringPrototypeIncludes = _primordials.StringPrototypeIncludes,
    StringPrototypeReplace = _primordials.StringPrototypeReplace,
    _Symbol = _primordials.Symbol,
    TypedArrayPrototypeIncludes = _primordials.TypedArrayPrototypeIncludes;

var _require = require('buffer'),
    Buffer = _require.Buffer;

var _require2 = require('internal/errors'),
    _require2$codes = _require2.codes,
    ERR_FS_EISDIR = _require2$codes.ERR_FS_EISDIR,
    ERR_FS_INVALID_SYMLINK_TYPE = _require2$codes.ERR_FS_INVALID_SYMLINK_TYPE,
    ERR_INVALID_ARG_TYPE = _require2$codes.ERR_INVALID_ARG_TYPE,
    ERR_INVALID_ARG_VALUE = _require2$codes.ERR_INVALID_ARG_VALUE,
    ERR_OUT_OF_RANGE = _require2$codes.ERR_OUT_OF_RANGE,
    hideStackFrames = _require2.hideStackFrames,
    uvException = _require2.uvException;

var _require3 = require('internal/util/types'),
    isArrayBufferView = _require3.isArrayBufferView,
    isUint8Array = _require3.isUint8Array,
    isDate = _require3.isDate,
    isBigUint64Array = _require3.isBigUint64Array;

var _require4 = require('internal/util'),
    once = _require4.once;

var _require5 = require('internal/url'),
    toPathIfFileURL = _require5.toPathIfFileURL;

var _require6 = require('internal/validators'),
    validateAbortSignal = _require6.validateAbortSignal,
    validateBoolean = _require6.validateBoolean,
    validateInt32 = _require6.validateInt32,
    validateInteger = _require6.validateInteger,
    validateObject = _require6.validateObject,
    validateUint32 = _require6.validateUint32;

var pathModule = require('path');

var kType = _Symbol('type');

var kStats = _Symbol('stats');

var assert = require('internal/assert');

var _internalBinding = internalBinding('constants'),
    _internalBinding$fs = _internalBinding.fs,
    _internalBinding$fs$F = _internalBinding$fs.F_OK,
    F_OK = _internalBinding$fs$F === void 0 ? 0 : _internalBinding$fs$F,
    _internalBinding$fs$W = _internalBinding$fs.W_OK,
    W_OK = _internalBinding$fs$W === void 0 ? 0 : _internalBinding$fs$W,
    _internalBinding$fs$R = _internalBinding$fs.R_OK,
    R_OK = _internalBinding$fs$R === void 0 ? 0 : _internalBinding$fs$R,
    _internalBinding$fs$X = _internalBinding$fs.X_OK,
    X_OK = _internalBinding$fs$X === void 0 ? 0 : _internalBinding$fs$X,
    COPYFILE_EXCL = _internalBinding$fs.COPYFILE_EXCL,
    COPYFILE_FICLONE = _internalBinding$fs.COPYFILE_FICLONE,
    COPYFILE_FICLONE_FORCE = _internalBinding$fs.COPYFILE_FICLONE_FORCE,
    O_APPEND = _internalBinding$fs.O_APPEND,
    O_CREAT = _internalBinding$fs.O_CREAT,
    O_EXCL = _internalBinding$fs.O_EXCL,
    O_RDONLY = _internalBinding$fs.O_RDONLY,
    O_RDWR = _internalBinding$fs.O_RDWR,
    O_SYNC = _internalBinding$fs.O_SYNC,
    O_TRUNC = _internalBinding$fs.O_TRUNC,
    O_WRONLY = _internalBinding$fs.O_WRONLY,
    S_IFBLK = _internalBinding$fs.S_IFBLK,
    S_IFCHR = _internalBinding$fs.S_IFCHR,
    S_IFDIR = _internalBinding$fs.S_IFDIR,
    S_IFIFO = _internalBinding$fs.S_IFIFO,
    S_IFLNK = _internalBinding$fs.S_IFLNK,
    S_IFMT = _internalBinding$fs.S_IFMT,
    S_IFREG = _internalBinding$fs.S_IFREG,
    S_IFSOCK = _internalBinding$fs.S_IFSOCK,
    UV_FS_SYMLINK_DIR = _internalBinding$fs.UV_FS_SYMLINK_DIR,
    UV_FS_SYMLINK_JUNCTION = _internalBinding$fs.UV_FS_SYMLINK_JUNCTION,
    UV_DIRENT_UNKNOWN = _internalBinding$fs.UV_DIRENT_UNKNOWN,
    UV_DIRENT_FILE = _internalBinding$fs.UV_DIRENT_FILE,
    UV_DIRENT_DIR = _internalBinding$fs.UV_DIRENT_DIR,
    UV_DIRENT_LINK = _internalBinding$fs.UV_DIRENT_LINK,
    UV_DIRENT_FIFO = _internalBinding$fs.UV_DIRENT_FIFO,
    UV_DIRENT_SOCKET = _internalBinding$fs.UV_DIRENT_SOCKET,
    UV_DIRENT_CHAR = _internalBinding$fs.UV_DIRENT_CHAR,
    UV_DIRENT_BLOCK = _internalBinding$fs.UV_DIRENT_BLOCK;
    // EISDIR = _internalBinding.os.errno.EISDIR; // The access modes can be any of F_OK, R_OK, W_OK or X_OK. Some might not be
// available on specific systems. They can be used in combination as well
// (F_OK | R_OK | W_OK | X_OK).


var kMinimumAccessMode = MathMin(F_OK, W_OK, R_OK, X_OK);
var kMaximumAccessMode = F_OK | W_OK | R_OK | X_OK;
var kDefaultCopyMode = 0; // The copy modes can be any of COPYFILE_EXCL, COPYFILE_FICLONE or
// COPYFILE_FICLONE_FORCE. They can be used in combination as well
// (COPYFILE_EXCL | COPYFILE_FICLONE | COPYFILE_FICLONE_FORCE).

var kMinimumCopyMode = MathMin(kDefaultCopyMode, COPYFILE_EXCL, COPYFILE_FICLONE, COPYFILE_FICLONE_FORCE);
var kMaximumCopyMode = COPYFILE_EXCL | COPYFILE_FICLONE | COPYFILE_FICLONE_FORCE; // Most platforms don't allow reads or writes >= 2 GB.
// See https://github.com/libuv/libuv/pull/1501.

var kIoMaxLength = Math.pow(2, 31) - 1; // Use 64kb in case the file type is not a regular file and thus do not know the
// actual file size. Increasing the value further results in more frequent over
// allocation for small files and consumes CPU time and memory that should be
// used else wise.
// Use up to 512kb per read otherwise to partition reading big files to prevent
// blocking other threads in case the available threads are all in use.

var kReadFileUnknownBufferLength = 64 * 1024;
var kReadFileBufferLength = 512 * 1024;
var kWriteFileMaxChunkSize = 512 * 1024;
var kMaxUserId = Math.pow(2, 32) - 1;
// var isWindows = process.platform === 'win32';
var fs;

function lazyLoadFs() {
  if (!fs) {
    fs = require('fs');
  }

  return fs;
}

function assertEncoding(encoding) {
  if (encoding && !Buffer.isEncoding(encoding)) {
    var reason = 'is invalid encoding';
    throw new ERR_INVALID_ARG_VALUE(encoding, 'encoding', reason);
  }
}

// var Dirent = /*#__PURE__*/function () {
//   function Dirent(name, type) {
//     _classCallCheck(this, Dirent);

//     this.name = name;
//     this[kType] = type;
//   }

//   _createClass(Dirent, [{
//     key: "isDirectory",
//     value: function isDirectory() {
//       return this[kType] === UV_DIRENT_DIR;
//     }
//   }, {
//     key: "isFile",
//     value: function isFile() {
//       return this[kType] === UV_DIRENT_FILE;
//     }
//   }, {
//     key: "isBlockDevice",
//     value: function isBlockDevice() {
//       return this[kType] === UV_DIRENT_BLOCK;
//     }
//   }, {
//     key: "isCharacterDevice",
//     value: function isCharacterDevice() {
//       return this[kType] === UV_DIRENT_CHAR;
//     }
//   }, {
//     key: "isSymbolicLink",
//     value: function isSymbolicLink() {
//       return this[kType] === UV_DIRENT_LINK;
//     }
//   }, {
//     key: "isFIFO",
//     value: function isFIFO() {
//       return this[kType] === UV_DIRENT_FIFO;
//     }
//   }, {
//     key: "isSocket",
//     value: function isSocket() {
//       return this[kType] === UV_DIRENT_SOCKET;
//     }
//   }]);

//   return Dirent;
// }();

// var DirentFromStats = /*#__PURE__*/function (_Dirent) {
//   _inherits(DirentFromStats, _Dirent);

//   var _super = _createSuper(DirentFromStats);

//   function DirentFromStats(name, stats) {
//     var _this;

//     _classCallCheck(this, DirentFromStats);

//     _this = _super.call(this, name, null);
//     _this[kStats] = stats;
//     return _this;
//   }

//   return DirentFromStats;
// }(Dirent);

// var _iterator = _createForOfIteratorHelper(ReflectOwnKeys(Dirent.prototype)),
//     _step;

// try {
//   var _loop = function _loop() {
//     var name = _step.value;

//     if (name === 'constructor') {
//       return "continue";
//     }

//     DirentFromStats.prototype[name] = function () {
//       return this[kStats][name]();
//     };
//   };

//   for (_iterator.s(); !(_step = _iterator.n()).done;) {
//     var _ret3 = _loop();

//     if (_ret3 === "continue") continue;
//   }
// } catch (err) {
//   _iterator.e(err);
// } finally {
//   _iterator.f();
// }

function copyObject(source) {
  var target = {};

  for (var key in source) {
    target[key] = source[key];
  }

  return target;
}

// var bufferSep = Buffer.from(pathModule.sep);

function join(path, name) {
  if ((typeof path === 'string' || isUint8Array(path)) && name === undefined) {
    return path;
  }

  if (typeof path === 'string' && isUint8Array(name)) {
    var pathBuffer = Buffer.from(pathModule.join(path, pathModule.sep));
    return Buffer.concat([pathBuffer, name]);
  }

  if (typeof path === 'string' && typeof name === 'string') {
    return pathModule.join(path, name);
  }

  if (isUint8Array(path) && isUint8Array(name)) {
    return Buffer.concat([path, bufferSep, name]);
  }

  throw new ERR_INVALID_ARG_TYPE('path', ['string', 'Buffer'], path);
}

function getDirents(path, _ref, callback) {
  var names = _ref[0],
      types = _ref[1];
  var i;

  if (typeof callback === 'function') {
    var _ret = function () {
      var len = names.length;
      var toFinish = 0;
      callback = once(callback);

      for (i = 0; i < len; i++) {
        var type = types[i];

        if (type === UV_DIRENT_UNKNOWN) {
          var _ret2 = function () {
            var name = names[i];
            var idx = i;
            toFinish++;
            var filepath = void 0;

            try {
              filepath = join(path, name);
            } catch (err) {
              callback(err);
              return {
                v: {
                  v: void 0
                }
              };
            }

            lazyLoadFs().lstat(filepath, function (err, stats) {
              if (err) {
                callback(err);
                return;
              }

              names[idx] = new DirentFromStats(name, stats);

              if (--toFinish === 0) {
                callback(null, names);
              }
            });
          }();

          if (_typeof(_ret2) === "object") return _ret2.v;
        } else {
          names[i] = new Dirent(names[i], types[i]);
        }
      }

      if (toFinish === 0) {
        callback(null, names);
      }
    }();

    if (_typeof(_ret) === "object") return _ret.v;
  } else {
    var len = names.length;

    for (i = 0; i < len; i++) {
      names[i] = getDirent(path, names[i], types[i]);
    }

    return names;
  }
}

function getDirent(path, name, type, callback) {
  if (typeof callback === 'function') {
    if (type === UV_DIRENT_UNKNOWN) {
      var filepath;

      try {
        filepath = join(path, name);
      } catch (err) {
        callback(err);
        return;
      }

      lazyLoadFs().lstat(filepath, function (err, stats) {
        if (err) {
          callback(err);
          return;
        }

        callback(null, new DirentFromStats(name, stats));
      });
    } else {
      callback(null, new Dirent(name, type));
    }
  } else if (type === UV_DIRENT_UNKNOWN) {
    var stats = lazyLoadFs().lstatSync(join(path, name));
    return new DirentFromStats(name, stats);
  } else {
    return new Dirent(name, type);
  }
}

function getOptions(options, defaultOptions) {
  if (options === null || options === undefined || typeof options === 'function') {
    return defaultOptions;
  }

  if (typeof options === 'string') {
    defaultOptions = _objectSpread({}, defaultOptions);
    defaultOptions.encoding = options;
    options = defaultOptions;
  } else if (_typeof(options) !== 'object') {
    throw new ERR_INVALID_ARG_TYPE('options', ['string', 'Object'], options);
  }

  if (options.encoding !== 'buffer') assertEncoding(options.encoding);

  if (options.signal !== undefined) {
    validateAbortSignal(options.signal, 'options.signal');
  }

  return options;
}
/**
 * @param {InternalFSBinding.FSSyncContext} ctx
 */


function handleErrorFromBinding(ctx) {
  if (ctx.errno !== undefined) {
    // libuv error numbers
    var err = uvException(ctx);
    ErrorCaptureStackTrace(err, handleErrorFromBinding);
    throw err;
  }

  if (ctx.error !== undefined) {
    // Errors created in C++ land.
    // TODO(joyeecheung): currently, ctx.error are encoding errors
    // usually caused by memory problems. We need to figure out proper error
    // code(s) for this.
    ErrorCaptureStackTrace(ctx.error, handleErrorFromBinding);
    throw ctx.error;
  }
} // Check if the path contains null types if it is a string nor Uint8Array,
// otherwise return silently.


var nullCheck = hideStackFrames(function (path, propName) {
  var throwError = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : true;
  var pathIsString = typeof path === 'string';
  var pathIsUint8Array = isUint8Array(path); // We can only perform meaningful checks on strings and Uint8Arrays.
  if (!pathIsString && !pathIsUint8Array || pathIsString && !StringPrototypeIncludes(path, "\0") || pathIsUint8Array && !TypedArrayPrototypeIncludes(path, 0)) {
    return;
  }
  var err = new ERR_INVALID_ARG_VALUE(propName, path, 'must be a string or Uint8Array without null bytes');
  if (throwError) {
    throw err;
  }

  return err;
});

function preprocessSymlinkDestination(path, type, linkPath) {
  if (!isWindows) {
    // No preprocessing is needed on Unix.
    return path;
  }

  path = '' + path;

  if (type === 'junction') {
    // Junctions paths need to be absolute and \\?\-prefixed.
    // A relative target is relative to the link's parent directory.
    path = pathModule.resolve(linkPath, '..', path);
    return pathModule.toNamespacedPath(path);
  }

  if (pathModule.isAbsolute(path)) {
    // If the path is absolute, use the \\?\-prefix to enable long filenames
    return pathModule.toNamespacedPath(path);
  } // Windows symlinks don't tolerate forward slashes.


  return StringPrototypeReplace(path, /\//g, '\\');
} // Constructor for file stats.


function StatsBase(dev, mode, nlink, uid, gid, rdev, blksize, ino, size, blocks) {
  this.dev = dev;
  this.mode = mode;
  this.nlink = nlink;
  this.uid = uid;
  this.gid = gid;
  this.rdev = rdev;
  this.blksize = blksize;
  this.ino = ino;
  this.size = size;
  this.blocks = blocks;
}

StatsBase.prototype.isDirectory = function () {
  return this._checkModeProperty(S_IFDIR);
};

StatsBase.prototype.isFile = function () {
  return this._checkModeProperty(S_IFREG);
};

StatsBase.prototype.isBlockDevice = function () {
  return this._checkModeProperty(S_IFBLK);
};

StatsBase.prototype.isCharacterDevice = function () {
  return this._checkModeProperty(S_IFCHR);
};

StatsBase.prototype.isSymbolicLink = function () {
  return this._checkModeProperty(S_IFLNK);
};

StatsBase.prototype.isFIFO = function () {
  return this._checkModeProperty(S_IFIFO);
};

StatsBase.prototype.isSocket = function () {
  return this._checkModeProperty(S_IFSOCK);
};

var kNsPerMsBigInt = Math.pow(10, 6);
var kNsPerSecBigInt = Math.pow(10, 9);
var kMsPerSec = Math.pow(10, 3);
var kNsPerMs = Math.pow(10, 6);

function msFromTimeSpec(sec, nsec) {
  return sec * kMsPerSec + nsec / kNsPerMs;
}

function nsFromTimeSpecBigInt(sec, nsec) {
  return sec * kNsPerSecBigInt + nsec;
} // The Date constructor performs Math.floor() to the timestamp.
// https://www.ecma-international.org/ecma-262/#sec-timeclip
// Since there may be a precision loss when the timestamp is
// converted to a floating point number, we manually round
// the timestamp here before passing it to Date().
// Refs: https://github.com/nodejs/node/pull/12607


function dateFromMs(ms) {
  return new Date(Number(ms) + 0.5);
}

function BigIntStats(dev, mode, nlink, uid, gid, rdev, blksize, ino, size, blocks, atimeNs, mtimeNs, ctimeNs, birthtimeNs) {
  ReflectApply(StatsBase, this, [dev, mode, nlink, uid, gid, rdev, blksize, ino, size, blocks]);
  this.atimeMs = atimeNs / kNsPerMsBigInt;
  this.mtimeMs = mtimeNs / kNsPerMsBigInt;
  this.ctimeMs = ctimeNs / kNsPerMsBigInt;
  this.birthtimeMs = birthtimeNs / kNsPerMsBigInt;
  this.atimeNs = atimeNs;
  this.mtimeNs = mtimeNs;
  this.ctimeNs = ctimeNs;
  this.birthtimeNs = birthtimeNs;
  this.atime = dateFromMs(this.atimeMs);
  this.mtime = dateFromMs(this.mtimeMs);
  this.ctime = dateFromMs(this.ctimeMs);
  this.birthtime = dateFromMs(this.birthtimeMs);
}

ObjectSetPrototypeOf(BigIntStats.prototype, StatsBase.prototype);
ObjectSetPrototypeOf(BigIntStats, StatsBase);

BigIntStats.prototype._checkModeProperty = function (property) {
  if (isWindows && (property === S_IFIFO || property === S_IFBLK || property === S_IFSOCK)) {
    return false; // Some types are not available on Windows
  }

  return (this.mode & BigInt(S_IFMT)) === BigInt(property);
};

function Stats(dev, mode, nlink, uid, gid, rdev, blksize, ino, size, blocks, atimeMs, mtimeMs, ctimeMs, birthtimeMs) {
  FunctionPrototypeCall(StatsBase, this, dev, mode, nlink, uid, gid, rdev, blksize, ino, size, blocks);
  this.atimeMs = atimeMs;
  this.mtimeMs = mtimeMs;
  this.ctimeMs = ctimeMs;
  this.birthtimeMs = birthtimeMs;
  this.atime = dateFromMs(atimeMs);
  this.mtime = dateFromMs(mtimeMs);
  this.ctime = dateFromMs(ctimeMs);
  this.birthtime = dateFromMs(birthtimeMs);
}

ObjectSetPrototypeOf(Stats.prototype, StatsBase.prototype);
ObjectSetPrototypeOf(Stats, StatsBase); // HACK: Workaround for https://github.com/standard-things/esm/issues/821.
// TODO(ronag): Remove this as soon as `esm` publishes a fixed version.

Stats.prototype.isFile = StatsBase.prototype.isFile;

Stats.prototype._checkModeProperty = function (property) {
  if (isWindows && (property === S_IFIFO || property === S_IFBLK || property === S_IFSOCK)) {
    return false; // Some types are not available on Windows
  }

  return (this.mode & S_IFMT) === property;
};
/**
 * @param {Float64Array | BigUint64Array} stats
 * @param {number} offset
 * @returns
 */


function getStatsFromBinding(stats) {
  var offset = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 0;

  if (isBigUint64Array(stats)) {
    return new BigIntStats(stats[0 + offset], stats[1 + offset], stats[2 + offset], stats[3 + offset], stats[4 + offset], stats[5 + offset], stats[6 + offset], stats[7 + offset], stats[8 + offset], stats[9 + offset], nsFromTimeSpecBigInt(stats[10 + offset], stats[11 + offset]), nsFromTimeSpecBigInt(stats[12 + offset], stats[13 + offset]), nsFromTimeSpecBigInt(stats[14 + offset], stats[15 + offset]), nsFromTimeSpecBigInt(stats[16 + offset], stats[17 + offset]));
  }

  return new Stats(stats[0 + offset], stats[1 + offset], stats[2 + offset], stats[3 + offset], stats[4 + offset], stats[5 + offset], stats[6 + offset], stats[7 + offset], stats[8 + offset], stats[9 + offset], msFromTimeSpec(stats[10 + offset], stats[11 + offset]), msFromTimeSpec(stats[12 + offset], stats[13 + offset]), msFromTimeSpec(stats[14 + offset], stats[15 + offset]), msFromTimeSpec(stats[16 + offset], stats[17 + offset]));
}

function stringToFlags(flags) {
  var name = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'flags';

  if (typeof flags === 'number') {
    validateInt32(flags, name);
    return flags;
  }

  if (flags == null) {
    return O_RDONLY;
  }

  switch (flags) {
    case 'r':
      return O_RDONLY;

    case 'rs': // Fall through.

    case 'sr':
      return O_RDONLY | O_SYNC;

    case 'r+':
      return O_RDWR;

    case 'rs+': // Fall through.

    case 'sr+':
      return O_RDWR | O_SYNC;

    case 'w':
      return O_TRUNC | O_CREAT | O_WRONLY;

    case 'wx': // Fall through.

    case 'xw':
      return O_TRUNC | O_CREAT | O_WRONLY | O_EXCL;

    case 'w+':
      return O_TRUNC | O_CREAT | O_RDWR;

    case 'wx+': // Fall through.

    case 'xw+':
      return O_TRUNC | O_CREAT | O_RDWR | O_EXCL;

    case 'a':
      return O_APPEND | O_CREAT | O_WRONLY;

    case 'ax': // Fall through.

    case 'xa':
      return O_APPEND | O_CREAT | O_WRONLY | O_EXCL;

    case 'as': // Fall through.

    case 'sa':
      return O_APPEND | O_CREAT | O_WRONLY | O_SYNC;

    case 'a+':
      return O_APPEND | O_CREAT | O_RDWR;

    case 'ax+': // Fall through.

    case 'xa+':
      return O_APPEND | O_CREAT | O_RDWR | O_EXCL;

    case 'as+': // Fall through.

    case 'sa+':
      return O_APPEND | O_CREAT | O_RDWR | O_SYNC;
  }

  throw new ERR_INVALID_ARG_VALUE('flags', flags);
}

var stringToSymlinkType = hideStackFrames(function (type) {
  var flags = 0;

  if (typeof type === 'string') {
    switch (type) {
      case 'dir':
        flags |= UV_FS_SYMLINK_DIR;
        break;

      case 'junction':
        flags |= UV_FS_SYMLINK_JUNCTION;
        break;

      case 'file':
        break;

      default:
        throw new ERR_FS_INVALID_SYMLINK_TYPE(type);
    }
  }

  return flags;
}); // converts Date or number to a fractional UNIX timestamp

function toUnixTimestamp(time) {
  var name = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'time';

  // eslint-disable-next-line eqeqeq
  if (typeof time === 'string' && +time == time) {
    return +time;
  }

  if (NumberIsFinite(time)) {
    if (time < 0) {
      return DateNow() / 1000;
    }

    return time;
  }

  if (isDate(time)) {
    // Convert to 123.456 UNIX timestamp
    return DatePrototypeGetTime(time) / 1000;
  }

  throw new ERR_INVALID_ARG_TYPE(name, ['Date', 'Time in seconds'], time);
}

var validateOffsetLengthRead = hideStackFrames(function (offset, length, bufferLength) {
  if (offset < 0) {
    throw new ERR_OUT_OF_RANGE('offset', '>= 0', offset);
  }

  if (length < 0) {
    throw new ERR_OUT_OF_RANGE('length', '>= 0', length);
  }

  if (offset + length > bufferLength) {
    throw new ERR_OUT_OF_RANGE('length', "<= ".concat(bufferLength - offset), length);
  }
});
var validateOffsetLengthWrite = hideStackFrames(function (offset, length, byteLength) {
  if (offset > byteLength) {
    throw new ERR_OUT_OF_RANGE('offset', "<= ".concat(byteLength), offset);
  }

  if (length > byteLength - offset) {
    throw new ERR_OUT_OF_RANGE('length', "<= ".concat(byteLength - offset), length);
  }

  if (length < 0) {
    throw new ERR_OUT_OF_RANGE('length', '>= 0', length);
  }

  validateInt32(length, 'length', 0);
});
var validatePath = hideStackFrames(function (path) {
  var propName = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'path';
  if (typeof path !== 'string' && !isUint8Array(path)) {
    throw new ERR_INVALID_ARG_TYPE(propName, ['string', 'Buffer', 'URL'], path);
  }
  var err = nullCheck(path, propName, false);
  if (err !== undefined) {
    throw err;
  }
});
var getValidatedPath = hideStackFrames(function (fileURLOrPath) {
  var propName = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'path';
  var path = toPathIfFileURL(fileURLOrPath);
  validatePath(path, propName);
  return path;
});
var getValidatedFd = hideStackFrames(function (fd) {
  var propName = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'fd';

  if (ObjectIs(fd, -0)) {
    return 0;
  }

  validateInt32(fd, propName, 0);
  return fd;
});
var validateBufferArray = hideStackFrames(function (buffers) {
  var propName = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'buffers';
  if (!ArrayIsArray(buffers)) throw new ERR_INVALID_ARG_TYPE(propName, 'ArrayBufferView[]', buffers);

  for (var i = 0; i < buffers.length; i++) {
    if (!isArrayBufferView(buffers[i])) throw new ERR_INVALID_ARG_TYPE(propName, 'ArrayBufferView[]', buffers);
  }

  return buffers;
});
var nonPortableTemplateWarn = true;

function warnOnNonPortableTemplate(template) {
  // Template strings passed to the mkdtemp() family of functions should not
  // end with 'X' because they are handled inconsistently across platforms.
  if (nonPortableTemplateWarn && StringPrototypeEndsWith(template, 'X')) {
    process.emitWarning('mkdtemp() templates ending with X are not portable. ' + 'For details see: https://nodejs.org/api/fs.html');
    nonPortableTemplateWarn = false;
  }
}

var defaultRmOptions = {
  recursive: false,
  force: false,
  retryDelay: 100,
  maxRetries: 0
};
var defaultRmdirOptions = {
  retryDelay: 100,
  maxRetries: 0,
  recursive: false
};
var validateRmOptions = hideStackFrames(function (path, options, expectDir, cb) {
  options = validateRmdirOptions(options, defaultRmOptions);
  validateBoolean(options.force, 'options.force');
  lazyLoadFs().stat(path, function (err, stats) {
    if (err) {
      if (options.force && err.code === 'ENOENT') {
        return cb(null, options);
      }

      return cb(err, options);
    }

    if (expectDir && !stats.isDirectory()) {
      return cb(false);
    }

    if (stats.isDirectory() && !options.recursive) {
      return cb(new ERR_FS_EISDIR({
        code: 'EISDIR',
        message: 'is a directory',
        path: path,
        syscall: 'rm',
        errno: EISDIR
      }));
    }

    return cb(null, options);
  });
});
var validateRmOptionsSync = hideStackFrames(function (path, options, expectDir) {
  options = validateRmdirOptions(options, defaultRmOptions);
  validateBoolean(options.force, 'options.force');

  if (!options.force || expectDir || !options.recursive) {
    var _lazyLoadFs$statSync;

    var isDirectory = (_lazyLoadFs$statSync = lazyLoadFs().statSync(path, {
      throwIfNoEntry: !options.force
    })) === null || _lazyLoadFs$statSync === void 0 ? void 0 : _lazyLoadFs$statSync.isDirectory();

    if (expectDir && !isDirectory) {
      return false;
    }

    if (isDirectory && !options.recursive) {
      throw new ERR_FS_EISDIR({
        code: 'EISDIR',
        message: 'is a directory',
        path: path,
        syscall: 'rm',
        errno: EISDIR
      });
    }
  }

  return options;
});
var recursiveRmdirWarned = process.noDeprecation;

function emitRecursiveRmdirWarning() {
  if (!recursiveRmdirWarned) {
    process.emitWarning('In future versions of Node.js, fs.rmdir(path, { recursive: true }) ' + 'will be removed. Use fs.rm(path, { recursive: true }) instead', 'DeprecationWarning', 'DEP0147');
    recursiveRmdirWarned = true;
  }
}

var validateRmdirOptions = hideStackFrames(function (options) {
  var defaults = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : defaultRmdirOptions;
  if (options === undefined) return defaults;
  validateObject(options, 'options');
  options = _objectSpread(_objectSpread({}, defaults), options);
  validateBoolean(options.recursive, 'options.recursive');
  validateInt32(options.retryDelay, 'options.retryDelay', 0);
  validateUint32(options.maxRetries, 'options.maxRetries');
  return options;
});
var getValidMode = hideStackFrames(function (mode, type) {
  var min = kMinimumAccessMode;
  var max = kMaximumAccessMode;
  var def = F_OK;

  if (type === 'copyFile') {
    min = kMinimumCopyMode;
    max = kMaximumCopyMode;
    def = mode || kDefaultCopyMode;
  } else {
    assert(type === 'access');
  }

  if (mode == null) {
    return def;
  }

  if (NumberIsInteger(mode) && mode >= min && mode <= max) {
    return mode;
  }

  if (typeof mode !== 'number') {
    throw new ERR_INVALID_ARG_TYPE('mode', 'integer', mode);
  }

  throw new ERR_OUT_OF_RANGE('mode', "an integer >= ".concat(min, " && <= ").concat(max), mode);
});
var validateStringAfterArrayBufferView = hideStackFrames(function (buffer, name) {
  if (typeof buffer === 'string') {
    return;
  }

  if (_typeof(buffer) === 'object' && buffer !== null && typeof buffer.toString === 'function' && ObjectPrototypeHasOwnProperty(buffer, 'toString')) {
    return;
  }

  throw new ERR_INVALID_ARG_TYPE(name, ['string', 'Buffer', 'TypedArray', 'DataView'], buffer);
});
var validatePosition = hideStackFrames(function (position, name) {
  if (typeof position === 'number') {
    validateInteger(position, 'position');
  } else if (typeof position === 'bigint') {
    if (!(position >= -Math.pow(2, 63) && position <= Math.pow(2, 63) - 1)) {
      throw new ERR_OUT_OF_RANGE('position', ">= ".concat(-Math.pow(2, 63), " && <= ").concat(Math.pow(2, 63) - 1), position);
    }
  } else {
    throw new ERR_INVALID_ARG_TYPE('position', ['integer', 'bigint'], position);
  }
});
module.exports = {
  constants: {
    kIoMaxLength: kIoMaxLength,
    kMaxUserId: kMaxUserId,
    kReadFileBufferLength: kReadFileBufferLength,
    kReadFileUnknownBufferLength: kReadFileUnknownBufferLength,
    kWriteFileMaxChunkSize: kWriteFileMaxChunkSize
  },
  assertEncoding: assertEncoding,
  BigIntStats: BigIntStats,
  // for testing
  copyObject: copyObject,
  // Dirent: Dirent,
  emitRecursiveRmdirWarning: emitRecursiveRmdirWarning,
  getDirent: getDirent,
  getDirents: getDirents,
  getOptions: getOptions,
  getValidatedFd: getValidatedFd,
  getValidatedPath: getValidatedPath,
  getValidMode: getValidMode,
  handleErrorFromBinding: handleErrorFromBinding,
  nullCheck: nullCheck,
  preprocessSymlinkDestination: preprocessSymlinkDestination,
  realpathCacheKey: _Symbol('realpathCacheKey'),
  getStatsFromBinding: getStatsFromBinding,
  stringToFlags: stringToFlags,
  stringToSymlinkType: stringToSymlinkType,
  Stats: Stats,
  toUnixTimestamp: toUnixTimestamp,
  validateBufferArray: validateBufferArray,
  validateOffsetLengthRead: validateOffsetLengthRead,
  validateOffsetLengthWrite: validateOffsetLengthWrite,
  validatePath: validatePath,
  validatePosition: validatePosition,
  validateRmOptions: validateRmOptions,
  validateRmOptionsSync: validateRmOptionsSync,
  validateRmdirOptions: validateRmdirOptions,
  validateStringAfterArrayBufferView: validateStringAfterArrayBufferView,
  warnOnNonPortableTemplate: warnOnNonPortableTemplate
};
