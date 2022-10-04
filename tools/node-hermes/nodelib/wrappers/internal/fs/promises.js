// @nolint
'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _toConsumableArray(arr) { return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread(); }

function _nonIterableSpread() { throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _iterableToArray(iter) { if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter); }

function _arrayWithoutHoles(arr) { if (Array.isArray(arr)) return _arrayLikeToArray(arr); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) { try { var info = gen[key](arg); var value = info.value; } catch (error) { reject(error); return; } if (info.done) { resolve(value); } else { Promise.resolve(value).then(_next, _throw); } }

function _asyncToGenerator(fn) { return function () { var self = this, args = arguments; return new Promise(function (resolve, reject) { var gen = fn.apply(self, args); function _next(value) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value); } function _throw(err) { asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err); } _next(undefined); }); }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _asyncIterator(iterable) { var method; if (typeof Symbol !== "undefined") { if (Symbol.asyncIterator) method = iterable[Symbol.asyncIterator]; if (method == null && Symbol.iterator) method = iterable[Symbol.iterator]; } if (method == null) method = iterable["@@asyncIterator"]; if (method == null) method = iterable["@@iterator"]; if (method == null) throw new TypeError("Object is not async iterable"); return method.call(iterable); }

var _primordials = primordials,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    Error = _primordials.Error,
    MathMax = _primordials.MathMax,
    MathMin = _primordials.MathMin,
    NumberIsSafeInteger = _primordials.NumberIsSafeInteger,
    _Promise = _primordials.Promise,
    PromisePrototypeThen = _primordials.PromisePrototypeThen,
    PromiseResolve = _primordials.PromiseResolve,
    PromiseReject = _primordials.PromiseReject,
    SafeArrayIterator = _primordials.SafeArrayIterator,
    SafePromisePrototypeFinally = _primordials.SafePromisePrototypeFinally,
    _Symbol = _primordials.Symbol,
    Uint8Array = _primordials.Uint8Array;
var _internalBinding$fs = internalBinding('constants').fs,
    F_OK = _internalBinding$fs.F_OK,
    O_SYMLINK = _internalBinding$fs.O_SYMLINK,
    O_WRONLY = _internalBinding$fs.O_WRONLY,
    S_IFMT = _internalBinding$fs.S_IFMT,
    S_IFREG = _internalBinding$fs.S_IFREG;
var binding = internalBinding('fs');

var _require = require('buffer'),
    Buffer = _require.Buffer;

var _require2 = require('internal/errors'),
    _require2$codes = _require2.codes,
    ERR_FS_FILE_TOO_LARGE = _require2$codes.ERR_FS_FILE_TOO_LARGE,
    ERR_INVALID_ARG_TYPE = _require2$codes.ERR_INVALID_ARG_TYPE,
    ERR_INVALID_ARG_VALUE = _require2$codes.ERR_INVALID_ARG_VALUE,
    ERR_METHOD_NOT_IMPLEMENTED = _require2$codes.ERR_METHOD_NOT_IMPLEMENTED,
    AbortError = _require2.AbortError,
    aggregateTwoErrors = _require2.aggregateTwoErrors;

var _require3 = require('internal/util/types'),
    isArrayBufferView = _require3.isArrayBufferView;

var _require4 = require('internal/fs/rimraf'),
    rimrafPromises = _require4.rimrafPromises;

var _require5 = require('internal/fs/utils'),
    _require5$constants = _require5.constants,
    kIoMaxLength = _require5$constants.kIoMaxLength,
    kMaxUserId = _require5$constants.kMaxUserId,
    kReadFileBufferLength = _require5$constants.kReadFileBufferLength,
    kReadFileUnknownBufferLength = _require5$constants.kReadFileUnknownBufferLength,
    kWriteFileMaxChunkSize = _require5$constants.kWriteFileMaxChunkSize,
    copyObject = _require5.copyObject,
    emitRecursiveRmdirWarning = _require5.emitRecursiveRmdirWarning,
    getDirents = _require5.getDirents,
    getOptions = _require5.getOptions,
    getStatsFromBinding = _require5.getStatsFromBinding,
    getValidatedPath = _require5.getValidatedPath,
    getValidMode = _require5.getValidMode,
    nullCheck = _require5.nullCheck,
    preprocessSymlinkDestination = _require5.preprocessSymlinkDestination,
    stringToFlags = _require5.stringToFlags,
    stringToSymlinkType = _require5.stringToSymlinkType,
    toUnixTimestamp = _require5.toUnixTimestamp,
    validateBufferArray = _require5.validateBufferArray,
    validateOffsetLengthRead = _require5.validateOffsetLengthRead,
    validateOffsetLengthWrite = _require5.validateOffsetLengthWrite,
    validateRmOptions = _require5.validateRmOptions,
    validateRmdirOptions = _require5.validateRmdirOptions,
    validateStringAfterArrayBufferView = _require5.validateStringAfterArrayBufferView,
    warnOnNonPortableTemplate = _require5.warnOnNonPortableTemplate;

var _require6 = require('internal/fs/dir'),
    opendir = _require6.opendir;

var _require7 = require('internal/validators'),
    parseFileMode = _require7.parseFileMode,
    validateAbortSignal = _require7.validateAbortSignal,
    validateBoolean = _require7.validateBoolean,
    validateBuffer = _require7.validateBuffer,
    validateEncoding = _require7.validateEncoding,
    validateInteger = _require7.validateInteger;

var pathModule = require('path');

var _require8 = require('internal/util'),
    promisify = _require8.promisify;

var _require9 = require('internal/event_target'),
    EventEmitterMixin = _require9.EventEmitterMixin;

var _require10 = require('internal/fs/watchers'),
    watch = _require10.watch;

var _require11 = require('internal/streams/utils'),
    isIterable = _require11.isIterable;

var assert = require('internal/assert');

var kHandle = _Symbol('kHandle');

var kFd = _Symbol('kFd');

var kRefs = _Symbol('kRefs');

var kClosePromise = _Symbol('kClosePromise');

var kCloseResolve = _Symbol('kCloseResolve');

var kCloseReject = _Symbol('kCloseReject');

var kRef = _Symbol('kRef');

var kUnref = _Symbol('kUnref');

var kUsePromises = binding.kUsePromises;

var _require12 = require('internal/worker/js_transferable'),
    JSTransferable = _require12.JSTransferable,
    kDeserialize = _require12.kDeserialize,
    kTransfer = _require12.kTransfer,
    kTransferList = _require12.kTransferList;

var getDirectoryEntriesPromise = promisify(getDirents);
var validateRmOptionsPromise = promisify(validateRmOptions);

var FileHandle = /*#__PURE__*/function (_EventEmitterMixin) {
  _inherits(FileHandle, _EventEmitterMixin);

  var _super = _createSuper(FileHandle);

  /**
   * @param {InternalFSBinding.FileHandle | undefined} filehandle
   */
  function FileHandle(filehandle) {
    var _this;

    _classCallCheck(this, FileHandle);

    _this = _super.call(this);

    _defineProperty(_assertThisInitialized(_this), "close", function () {
      if (_this[kFd] === -1) {
        return PromiseResolve();
      }

      if (_this[kClosePromise]) {
        return _this[kClosePromise];
      }

      _this[kRefs]--;

      if (_this[kRefs] === 0) {
        _this[kFd] = -1;
        _this[kClosePromise] = SafePromisePrototypeFinally(_this[kHandle].close(), function () {
          _this[kClosePromise] = undefined;
        });
      } else {
        _this[kClosePromise] = SafePromisePrototypeFinally(new _Promise(function (resolve, reject) {
          _this[kCloseResolve] = resolve;
          _this[kCloseReject] = reject;
        }), function () {
          _this[kClosePromise] = undefined;
          _this[kCloseReject] = undefined;
          _this[kCloseResolve] = undefined;
        });
      }

      _this.emit('close');

      return _this[kClosePromise];
    });

    _this[kHandle] = filehandle;
    _this[kFd] = filehandle ? filehandle.fd : -1;
    _this[kRefs] = 1;
    _this[kClosePromise] = null;
    return _this;
  }

  _createClass(FileHandle, [{
    key: "getAsyncId",
    value: function getAsyncId() {
      return this[kHandle].getAsyncId();
    }
  }, {
    key: "fd",
    get: function get() {
      return this[kFd];
    }
  }, {
    key: "appendFile",
    value: function appendFile(data, options) {
      return fsCall(_writeFile, this, data, options);
    }
  }, {
    key: "chmod",
    value: function chmod(mode) {
      return fsCall(fchmod, this, mode);
    }
  }, {
    key: "chown",
    value: function chown(uid, gid) {
      return fsCall(fchown, this, uid, gid);
    }
  }, {
    key: "datasync",
    value: function datasync() {
      return fsCall(fdatasync, this);
    }
  }, {
    key: "sync",
    value: function sync() {
      return fsCall(fsync, this);
    }
  }, {
    key: "read",
    value: function read(buffer, offset, length, position) {
      return fsCall(_read, this, buffer, offset, length, position);
    }
  }, {
    key: "readv",
    value: function readv(buffers, position) {
      return fsCall(_readv, this, buffers, position);
    }
  }, {
    key: "readFile",
    value: function readFile(options) {
      return fsCall(_readFile, this, options);
    }
  }, {
    key: "stat",
    value: function stat(options) {
      return fsCall(fstat, this, options);
    }
  }, {
    key: "truncate",
    value: function truncate() {
      var len = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;
      return fsCall(ftruncate, this, len);
    }
  }, {
    key: "utimes",
    value: function utimes(atime, mtime) {
      return fsCall(futimes, this, atime, mtime);
    }
  }, {
    key: "write",
    value: function write(buffer, offset, length, position) {
      return fsCall(_write, this, buffer, offset, length, position);
    }
  }, {
    key: "writev",
    value: function writev(buffers, position) {
      return fsCall(_writev, this, buffers, position);
    }
  }, {
    key: "writeFile",
    value: function writeFile(data, options) {
      return fsCall(_writeFile, this, data, options);
    }
  }, {
    key: kTransfer,
    value: function value() {
      if (this[kClosePromise] || this[kRefs] > 1) {
        var DOMException = internalBinding('messaging').DOMException;
        throw new DOMException('Cannot transfer FileHandle while in use', 'DataCloneError');
      }

      var handle = this[kHandle];
      this[kFd] = -1;
      this[kHandle] = null;
      this[kRefs] = 0;
      return {
        data: {
          handle: handle
        },
        deserializeInfo: 'internal/fs/promises:FileHandle'
      };
    }
  }, {
    key: kTransferList,
    value: function value() {
      return [this[kHandle]];
    }
  }, {
    key: kDeserialize,
    value: function value(_ref) {
      var handle = _ref.handle;
      this[kHandle] = handle;
      this[kFd] = handle.fd;
    }
  }, {
    key: kRef,
    value: function value() {
      this[kRefs]++;
    }
  }, {
    key: kUnref,
    value: function value() {
      this[kRefs]--;

      if (this[kRefs] === 0) {
        this[kFd] = -1;
        PromisePrototypeThen(this[kHandle].close(), this[kCloseResolve], this[kCloseReject]);
      }
    }
  }]);

  return FileHandle;
}(EventEmitterMixin(JSTransferable));

function handleFdClose(_x, _x2) {
  return _handleFdClose.apply(this, arguments);
}

function _handleFdClose() {
  _handleFdClose = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee(fileOpPromise, closeFunc) {
    return regeneratorRuntime.wrap(function _callee$(_context2) {
      while (1) {
        switch (_context2.prev = _context2.next) {
          case 0:
            return _context2.abrupt("return", PromisePrototypeThen(fileOpPromise, function (result) {
              return PromisePrototypeThen(closeFunc(), function () {
                return result;
              });
            }, function (opError) {
              return PromisePrototypeThen(closeFunc(), function () {
                return PromiseReject(opError);
              }, function (closeError) {
                return PromiseReject(aggregateTwoErrors(closeError, opError));
              });
            }));

          case 1:
          case "end":
            return _context2.stop();
        }
      }
    }, _callee);
  }));
  return _handleFdClose.apply(this, arguments);
}

function fsCall(_x3, _x4) {
  return _fsCall.apply(this, arguments);
}

function _fsCall() {
  _fsCall = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee2(fn, handle) {
    var err,
        _len,
        args,
        _key,
        _args3 = arguments;

    return regeneratorRuntime.wrap(function _callee2$(_context3) {
      while (1) {
        switch (_context3.prev = _context3.next) {
          case 0:
            assert(handle[kRefs] !== undefined, 'handle must be an instance of FileHandle');

            if (!(handle.fd === -1)) {
              _context3.next = 6;
              break;
            }

            // eslint-disable-next-line no-restricted-syntax
            err = new Error('file closed');
            err.code = 'EBADF';
            err.syscall = fn.name;
            throw err;

          case 6:
            _context3.prev = 6;
            handle[kRef]();

            for (_len = _args3.length, args = new Array(_len > 2 ? _len - 2 : 0), _key = 2; _key < _len; _key++) {
              args[_key - 2] = _args3[_key];
            }

            _context3.next = 11;
            return fn.apply(void 0, [handle].concat(_toConsumableArray(new SafeArrayIterator(args))));

          case 11:
            return _context3.abrupt("return", _context3.sent);

          case 12:
            _context3.prev = 12;
            handle[kUnref]();
            return _context3.finish(12);

          case 15:
          case "end":
            return _context3.stop();
        }
      }
    }, _callee2, null, [[6,, 12, 15]]);
  }));
  return _fsCall.apply(this, arguments);
}

function checkAborted(signal) {
  if (signal !== null && signal !== void 0 && signal.aborted) throw new AbortError();
}

function writeFileHandle(filehandle, data, signal, encoding) {
  var _iteratorNormalCompletion, _didIteratorError, _iteratorError, _iterator, _step, _value, buf, toWrite, _remaining, writeSize, _await$write, bytesWritten, remaining, _await$write2, _bytesWritten;

  return regeneratorRuntime.async(function writeFileHandle$(_context) {
    while (1) {
      switch (_context.prev = _context.next) {
        case 0:
          checkAborted(signal);

          if (!isCustomIterable(data)) {
            _context.next = 48;
            break;
          }

          _iteratorNormalCompletion = true;
          _didIteratorError = false;
          _context.prev = 4;
          _iterator = _asyncIterator(data);

        case 6:
          _context.next = 8;
          return regeneratorRuntime.awrap(_iterator.next());

        case 8:
          _step = _context.sent;
          _iteratorNormalCompletion = _step.done;
          _context.next = 12;
          return regeneratorRuntime.awrap(_step.value);

        case 12:
          _value = _context.sent;

          if (_iteratorNormalCompletion) {
            _context.next = 31;
            break;
          }

          buf = _value;
          checkAborted(signal);
          toWrite = isArrayBufferView(buf) ? buf : Buffer.from(buf, encoding || 'utf8');
          _remaining = toWrite.byteLength;

        case 18:
          if (!(_remaining > 0)) {
            _context.next = 28;
            break;
          }

          writeSize = MathMin(kWriteFileMaxChunkSize, _remaining);
          _context.next = 22;
          return regeneratorRuntime.awrap(_write(filehandle, toWrite, toWrite.byteLength - _remaining, writeSize));

        case 22:
          _await$write = _context.sent;
          bytesWritten = _await$write.bytesWritten;
          _remaining -= bytesWritten;
          checkAborted(signal);
          _context.next = 18;
          break;

        case 28:
          _iteratorNormalCompletion = true;
          _context.next = 6;
          break;

        case 31:
          _context.next = 37;
          break;

        case 33:
          _context.prev = 33;
          _context.t0 = _context["catch"](4);
          _didIteratorError = true;
          _iteratorError = _context.t0;

        case 37:
          _context.prev = 37;
          _context.prev = 38;

          if (!(!_iteratorNormalCompletion && _iterator["return"] != null)) {
            _context.next = 42;
            break;
          }

          _context.next = 42;
          return regeneratorRuntime.awrap(_iterator["return"]());

        case 42:
          _context.prev = 42;

          if (!_didIteratorError) {
            _context.next = 45;
            break;
          }

          throw _iteratorError;

        case 45:
          return _context.finish(42);

        case 46:
          return _context.finish(37);

        case 47:
          return _context.abrupt("return");

        case 48:
          data = new Uint8Array(data.buffer, data.byteOffset, data.byteLength);
          remaining = data.byteLength;

          if (!(remaining === 0)) {
            _context.next = 52;
            break;
          }

          return _context.abrupt("return");

        case 52:
          checkAborted(signal);
          _context.next = 55;
          return regeneratorRuntime.awrap(_write(filehandle, data, 0, MathMin(kWriteFileMaxChunkSize, data.byteLength)));

        case 55:
          _await$write2 = _context.sent;
          _bytesWritten = _await$write2.bytesWritten;
          remaining -= _bytesWritten;
          data = new Uint8Array(data.buffer, data.byteOffset + _bytesWritten, data.byteLength - _bytesWritten);

        case 59:
          if (remaining > 0) {
            _context.next = 52;
            break;
          }

        case 60:
        case "end":
          return _context.stop();
      }
    }
  }, null, null, [[4, 33, 37, 47], [38,, 42, 46]], Promise);
}

function readFileHandle(_x5, _x6) {
  return _readFileHandle.apply(this, arguments);
} // All of the functions are defined as async in order to ensure that errors
// thrown cause promise rejections rather than being thrown synchronously.


function _readFileHandle() {
  _readFileHandle = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee3(filehandle, options) {
    var signal, statFields, size, endOfFile, totalRead, noSize, buffers, fullBuffer, buffer, offset, length, bytesRead, isBufferFull, chunkBuffer, result;
    return regeneratorRuntime.wrap(function _callee3$(_context4) {
      while (1) {
        switch (_context4.prev = _context4.next) {
          case 0:
            signal = options === null || options === void 0 ? void 0 : options.signal;
            checkAborted(signal);
            _context4.next = 4;
            return binding.fstat(filehandle.fd, false, kUsePromises);

          case 4:
            statFields = _context4.sent;
            checkAborted(signal);

            if ((statFields[1
            /* mode */
            ] & S_IFMT) === S_IFREG) {
              size = statFields[8
              /* size */
              ];
            } else {
              size = 0;
            }

            if (!(size > kIoMaxLength)) {
              _context4.next = 9;
              break;
            }

            throw new ERR_FS_FILE_TOO_LARGE(size);

          case 9:
            endOfFile = false;
            totalRead = 0;
            noSize = size === 0;
            buffers = [];
            fullBuffer = noSize ? undefined : Buffer.allocUnsafeSlow(size);

          case 14:
            checkAborted(signal);
            buffer = void 0;
            offset = void 0;
            length = void 0;

            if (noSize) {
              buffer = Buffer.allocUnsafeSlow(kReadFileUnknownBufferLength);
              offset = 0;
              length = kReadFileUnknownBufferLength;
            } else {
              buffer = fullBuffer;
              offset = totalRead;
              length = MathMin(size - totalRead, kReadFileBufferLength);
            }

            _context4.next = 21;
            return binding.read(filehandle.fd, buffer, offset, length, -1, kUsePromises);

          case 21:
            _context4.t0 = _context4.sent;

            if (_context4.t0) {
              _context4.next = 24;
              break;
            }

            _context4.t0 = 0;

          case 24:
            bytesRead = _context4.t0;
            totalRead += bytesRead;
            endOfFile = bytesRead === 0 || totalRead === size;

            if (noSize && bytesRead > 0) {
              isBufferFull = bytesRead === kReadFileUnknownBufferLength;
              chunkBuffer = isBufferFull ? buffer : buffer.slice(0, bytesRead);
              ArrayPrototypePush(buffers, chunkBuffer);
            }

          case 28:
            if (!endOfFile) {
              _context4.next = 14;
              break;
            }

          case 29:
            if (size > 0) {
              result = totalRead === size ? fullBuffer : fullBuffer.slice(0, totalRead);
            } else {
              result = buffers.length === 1 ? buffers[0] : Buffer.concat(buffers, totalRead);
            }

            return _context4.abrupt("return", options.encoding ? result.toString(options.encoding) : result);

          case 31:
          case "end":
            return _context4.stop();
        }
      }
    }, _callee3);
  }));
  return _readFileHandle.apply(this, arguments);
}

function access(_x7) {
  return _access.apply(this, arguments);
}

function _access() {
  _access = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee4(path) {
    var mode,
        _args5 = arguments;
    return regeneratorRuntime.wrap(function _callee4$(_context5) {
      while (1) {
        switch (_context5.prev = _context5.next) {
          case 0:
            mode = _args5.length > 1 && _args5[1] !== undefined ? _args5[1] : F_OK;
            path = getValidatedPath(path);
            mode = getValidMode(mode, 'access');
            return _context5.abrupt("return", binding.access(pathModule.toNamespacedPath(path), mode, kUsePromises));

          case 4:
          case "end":
            return _context5.stop();
        }
      }
    }, _callee4);
  }));
  return _access.apply(this, arguments);
}

function copyFile(_x8, _x9, _x10) {
  return _copyFile.apply(this, arguments);
} // Note that unlike fs.open() which uses numeric file descriptors,
// fsPromises.open() uses the fs.FileHandle class.


function _copyFile() {
  _copyFile = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee5(src, dest, mode) {
    return regeneratorRuntime.wrap(function _callee5$(_context6) {
      while (1) {
        switch (_context6.prev = _context6.next) {
          case 0:
            src = getValidatedPath(src, 'src');
            dest = getValidatedPath(dest, 'dest');
            mode = getValidMode(mode, 'copyFile');
            return _context6.abrupt("return", binding.copyFile(pathModule.toNamespacedPath(src), pathModule.toNamespacedPath(dest), mode, kUsePromises));

          case 4:
          case "end":
            return _context6.stop();
        }
      }
    }, _callee5);
  }));
  return _copyFile.apply(this, arguments);
}

function open(_x11, _x12, _x13) {
  return _open.apply(this, arguments);
}

function _open() {
  _open = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee6(path, flags, mode) {
    var flagsNumber;
    return regeneratorRuntime.wrap(function _callee6$(_context7) {
      while (1) {
        switch (_context7.prev = _context7.next) {
          case 0:
            path = getValidatedPath(path);
            flagsNumber = stringToFlags(flags);
            mode = parseFileMode(mode, 'mode', 438);
            _context7.t0 = FileHandle;
            _context7.next = 6;
            return binding.openFileHandle(pathModule.toNamespacedPath(path), flagsNumber, mode, kUsePromises);

          case 6:
            _context7.t1 = _context7.sent;
            return _context7.abrupt("return", new _context7.t0(_context7.t1));

          case 8:
          case "end":
            return _context7.stop();
        }
      }
    }, _callee6);
  }));
  return _open.apply(this, arguments);
}

function _read(_x14, _x15, _x16, _x17, _x18) {
  return _read2.apply(this, arguments);
}

function _read2() {
  _read2 = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee7(handle, bufferOrOptions, offset, length, position) {
    var buffer, bytesRead;
    return regeneratorRuntime.wrap(function _callee7$(_context8) {
      while (1) {
        switch (_context8.prev = _context8.next) {
          case 0:
            buffer = bufferOrOptions;

            if (!isArrayBufferView(buffer)) {
              if (bufferOrOptions === undefined) {
                bufferOrOptions = {};
              }

              if (bufferOrOptions.buffer) {
                buffer = bufferOrOptions.buffer;
                validateBuffer(buffer);
              } else {
                buffer = Buffer.alloc(16384);
              }

              offset = bufferOrOptions.offset || 0;
              length = buffer.byteLength;
              position = bufferOrOptions.position || null;
            }

            if (offset == null) {
              offset = 0;
            } else {
              validateInteger(offset, 'offset', 0);
            }

            length |= 0;

            if (!(length === 0)) {
              _context8.next = 6;
              break;
            }

            return _context8.abrupt("return", {
              bytesRead: length,
              buffer: buffer
            });

          case 6:
            if (!(buffer.byteLength === 0)) {
              _context8.next = 8;
              break;
            }

            throw new ERR_INVALID_ARG_VALUE('buffer', buffer, 'is empty and cannot be written');

          case 8:
            validateOffsetLengthRead(offset, length, buffer.byteLength);
            if (!NumberIsSafeInteger(position)) position = -1;
            _context8.next = 12;
            return binding.read(handle.fd, buffer, offset, length, position, kUsePromises);

          case 12:
            _context8.t0 = _context8.sent;

            if (_context8.t0) {
              _context8.next = 15;
              break;
            }

            _context8.t0 = 0;

          case 15:
            bytesRead = _context8.t0;
            return _context8.abrupt("return", {
              bytesRead: bytesRead,
              buffer: buffer
            });

          case 17:
          case "end":
            return _context8.stop();
        }
      }
    }, _callee7);
  }));
  return _read2.apply(this, arguments);
}

function _readv(_x19, _x20, _x21) {
  return _readv2.apply(this, arguments);
}

function _readv2() {
  _readv2 = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee8(handle, buffers, position) {
    var bytesRead;
    return regeneratorRuntime.wrap(function _callee8$(_context9) {
      while (1) {
        switch (_context9.prev = _context9.next) {
          case 0:
            validateBufferArray(buffers);
            if (typeof position !== 'number') position = null;
            _context9.next = 4;
            return binding.readBuffers(handle.fd, buffers, position, kUsePromises);

          case 4:
            _context9.t0 = _context9.sent;

            if (_context9.t0) {
              _context9.next = 7;
              break;
            }

            _context9.t0 = 0;

          case 7:
            bytesRead = _context9.t0;
            return _context9.abrupt("return", {
              bytesRead: bytesRead,
              buffers: buffers
            });

          case 9:
          case "end":
            return _context9.stop();
        }
      }
    }, _callee8);
  }));
  return _readv2.apply(this, arguments);
}

function _write(_x22, _x23, _x24, _x25, _x26) {
  return _write2.apply(this, arguments);
}

function _write2() {
  _write2 = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee9(handle, buffer, offset, length, position) {
    var _bytesWritten2, bytesWritten;

    return regeneratorRuntime.wrap(function _callee9$(_context10) {
      while (1) {
        switch (_context10.prev = _context10.next) {
          case 0:
            if (!((buffer === null || buffer === void 0 ? void 0 : buffer.byteLength) === 0)) {
              _context10.next = 2;
              break;
            }

            return _context10.abrupt("return", {
              bytesWritten: 0,
              buffer: buffer
            });

          case 2:
            if (!isArrayBufferView(buffer)) {
              _context10.next = 14;
              break;
            }

            if (offset == null) {
              offset = 0;
            } else {
              validateInteger(offset, 'offset', 0);
            }

            if (typeof length !== 'number') length = buffer.byteLength - offset;
            if (typeof position !== 'number') position = null;
            validateOffsetLengthWrite(offset, length, buffer.byteLength);
            _context10.next = 9;
            return binding.writeBuffer(handle.fd, buffer, offset, length, position, kUsePromises);

          case 9:
            _context10.t0 = _context10.sent;

            if (_context10.t0) {
              _context10.next = 12;
              break;
            }

            _context10.t0 = 0;

          case 12:
            _bytesWritten2 = _context10.t0;
            return _context10.abrupt("return", {
              bytesWritten: _bytesWritten2,
              buffer: buffer
            });

          case 14:
            validateStringAfterArrayBufferView(buffer, 'buffer');
            validateEncoding(buffer, length);
            _context10.next = 18;
            return binding.writeString(handle.fd, buffer, offset, length, kUsePromises);

          case 18:
            _context10.t1 = _context10.sent;

            if (_context10.t1) {
              _context10.next = 21;
              break;
            }

            _context10.t1 = 0;

          case 21:
            bytesWritten = _context10.t1;
            return _context10.abrupt("return", {
              bytesWritten: bytesWritten,
              buffer: buffer
            });

          case 23:
          case "end":
            return _context10.stop();
        }
      }
    }, _callee9);
  }));
  return _write2.apply(this, arguments);
}

function _writev(_x27, _x28, _x29) {
  return _writev2.apply(this, arguments);
}

function _writev2() {
  _writev2 = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee10(handle, buffers, position) {
    var bytesWritten;
    return regeneratorRuntime.wrap(function _callee10$(_context11) {
      while (1) {
        switch (_context11.prev = _context11.next) {
          case 0:
            validateBufferArray(buffers);
            if (typeof position !== 'number') position = null;
            _context11.next = 4;
            return binding.writeBuffers(handle.fd, buffers, position, kUsePromises);

          case 4:
            _context11.t0 = _context11.sent;

            if (_context11.t0) {
              _context11.next = 7;
              break;
            }

            _context11.t0 = 0;

          case 7:
            bytesWritten = _context11.t0;
            return _context11.abrupt("return", {
              bytesWritten: bytesWritten,
              buffers: buffers
            });

          case 9:
          case "end":
            return _context11.stop();
        }
      }
    }, _callee10);
  }));
  return _writev2.apply(this, arguments);
}

function rename(_x30, _x31) {
  return _rename.apply(this, arguments);
}

function _rename() {
  _rename = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee11(oldPath, newPath) {
    return regeneratorRuntime.wrap(function _callee11$(_context12) {
      while (1) {
        switch (_context12.prev = _context12.next) {
          case 0:
            oldPath = getValidatedPath(oldPath, 'oldPath');
            newPath = getValidatedPath(newPath, 'newPath');
            return _context12.abrupt("return", binding.rename(pathModule.toNamespacedPath(oldPath), pathModule.toNamespacedPath(newPath), kUsePromises));

          case 3:
          case "end":
            return _context12.stop();
        }
      }
    }, _callee11);
  }));
  return _rename.apply(this, arguments);
}

function truncate(_x32) {
  return _truncate.apply(this, arguments);
}

function _truncate() {
  _truncate = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee12(path) {
    var len,
        fd,
        _args13 = arguments;
    return regeneratorRuntime.wrap(function _callee12$(_context13) {
      while (1) {
        switch (_context13.prev = _context13.next) {
          case 0:
            len = _args13.length > 1 && _args13[1] !== undefined ? _args13[1] : 0;
            _context13.next = 3;
            return open(path, 'r+');

          case 3:
            fd = _context13.sent;
            return _context13.abrupt("return", handleFdClose(ftruncate(fd, len), fd.close));

          case 5:
          case "end":
            return _context13.stop();
        }
      }
    }, _callee12);
  }));
  return _truncate.apply(this, arguments);
}

function ftruncate(_x33) {
  return _ftruncate.apply(this, arguments);
}

function _ftruncate() {
  _ftruncate = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee13(handle) {
    var len,
        _args14 = arguments;
    return regeneratorRuntime.wrap(function _callee13$(_context14) {
      while (1) {
        switch (_context14.prev = _context14.next) {
          case 0:
            len = _args14.length > 1 && _args14[1] !== undefined ? _args14[1] : 0;
            validateInteger(len, 'len');
            len = MathMax(0, len);
            return _context14.abrupt("return", binding.ftruncate(handle.fd, len, kUsePromises));

          case 4:
          case "end":
            return _context14.stop();
        }
      }
    }, _callee13);
  }));
  return _ftruncate.apply(this, arguments);
}

function rm(_x34, _x35) {
  return _rm.apply(this, arguments);
}

function _rm() {
  _rm = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee14(path, options) {
    return regeneratorRuntime.wrap(function _callee14$(_context15) {
      while (1) {
        switch (_context15.prev = _context15.next) {
          case 0:
            path = pathModule.toNamespacedPath(getValidatedPath(path));
            _context15.next = 3;
            return validateRmOptionsPromise(path, options, false);

          case 3:
            options = _context15.sent;
            return _context15.abrupt("return", rimrafPromises(path, options));

          case 5:
          case "end":
            return _context15.stop();
        }
      }
    }, _callee14);
  }));
  return _rm.apply(this, arguments);
}

function rmdir(_x36, _x37) {
  return _rmdir.apply(this, arguments);
}

function _rmdir() {
  _rmdir = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee15(path, options) {
    var stats;
    return regeneratorRuntime.wrap(function _callee15$(_context16) {
      while (1) {
        switch (_context16.prev = _context16.next) {
          case 0:
            path = pathModule.toNamespacedPath(getValidatedPath(path));
            options = validateRmdirOptions(options);

            if (!options.recursive) {
              _context16.next = 9;
              break;
            }

            emitRecursiveRmdirWarning();
            _context16.next = 6;
            return stat(path);

          case 6:
            stats = _context16.sent;

            if (!stats.isDirectory()) {
              _context16.next = 9;
              break;
            }

            return _context16.abrupt("return", rimrafPromises(path, options));

          case 9:
            return _context16.abrupt("return", binding.rmdir(path, kUsePromises));

          case 10:
          case "end":
            return _context16.stop();
        }
      }
    }, _callee15);
  }));
  return _rmdir.apply(this, arguments);
}

function fdatasync(_x38) {
  return _fdatasync.apply(this, arguments);
}

function _fdatasync() {
  _fdatasync = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee16(handle) {
    return regeneratorRuntime.wrap(function _callee16$(_context17) {
      while (1) {
        switch (_context17.prev = _context17.next) {
          case 0:
            return _context17.abrupt("return", binding.fdatasync(handle.fd, kUsePromises));

          case 1:
          case "end":
            return _context17.stop();
        }
      }
    }, _callee16);
  }));
  return _fdatasync.apply(this, arguments);
}

function fsync(_x39) {
  return _fsync.apply(this, arguments);
}

function _fsync() {
  _fsync = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee17(handle) {
    return regeneratorRuntime.wrap(function _callee17$(_context18) {
      while (1) {
        switch (_context18.prev = _context18.next) {
          case 0:
            return _context18.abrupt("return", binding.fsync(handle.fd, kUsePromises));

          case 1:
          case "end":
            return _context18.stop();
        }
      }
    }, _callee17);
  }));
  return _fsync.apply(this, arguments);
}

function mkdir(_x40, _x41) {
  return _mkdir.apply(this, arguments);
}

function _mkdir() {
  _mkdir = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee18(path, options) {
    var _ref2, _ref2$recursive, recursive, _ref2$mode, mode;

    return regeneratorRuntime.wrap(function _callee18$(_context19) {
      while (1) {
        switch (_context19.prev = _context19.next) {
          case 0:
            if (typeof options === 'number' || typeof options === 'string') {
              options = {
                mode: options
              };
            }

            _ref2 = options || {}, _ref2$recursive = _ref2.recursive, recursive = _ref2$recursive === void 0 ? false : _ref2$recursive, _ref2$mode = _ref2.mode, mode = _ref2$mode === void 0 ? 511 : _ref2$mode;
            path = getValidatedPath(path);
            validateBoolean(recursive, 'options.recursive');
            return _context19.abrupt("return", binding.mkdir(pathModule.toNamespacedPath(path), parseFileMode(mode, 'mode', 511), recursive, kUsePromises));

          case 5:
          case "end":
            return _context19.stop();
        }
      }
    }, _callee18);
  }));
  return _mkdir.apply(this, arguments);
}

function readdir(_x42, _x43) {
  return _readdir.apply(this, arguments);
}

function _readdir() {
  _readdir = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee19(path, options) {
    var result;
    return regeneratorRuntime.wrap(function _callee19$(_context20) {
      while (1) {
        switch (_context20.prev = _context20.next) {
          case 0:
            options = getOptions(options, {});
            path = getValidatedPath(path);
            _context20.next = 4;
            return binding.readdir(pathModule.toNamespacedPath(path), options.encoding, !!options.withFileTypes, kUsePromises);

          case 4:
            result = _context20.sent;
            return _context20.abrupt("return", options.withFileTypes ? getDirectoryEntriesPromise(path, result) : result);

          case 6:
          case "end":
            return _context20.stop();
        }
      }
    }, _callee19);
  }));
  return _readdir.apply(this, arguments);
}

function readlink(_x44, _x45) {
  return _readlink.apply(this, arguments);
}

function _readlink() {
  _readlink = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee20(path, options) {
    return regeneratorRuntime.wrap(function _callee20$(_context21) {
      while (1) {
        switch (_context21.prev = _context21.next) {
          case 0:
            options = getOptions(options, {});
            path = getValidatedPath(path, 'oldPath');
            return _context21.abrupt("return", binding.readlink(pathModule.toNamespacedPath(path), options.encoding, kUsePromises));

          case 3:
          case "end":
            return _context21.stop();
        }
      }
    }, _callee20);
  }));
  return _readlink.apply(this, arguments);
}

function symlink(_x46, _x47, _x48) {
  return _symlink.apply(this, arguments);
}

function _symlink() {
  _symlink = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee21(target, path, type_) {
    var type;
    return regeneratorRuntime.wrap(function _callee21$(_context22) {
      while (1) {
        switch (_context22.prev = _context22.next) {
          case 0:
            type = typeof type_ === 'string' ? type_ : null;
            target = getValidatedPath(target, 'target');
            path = getValidatedPath(path);
            return _context22.abrupt("return", binding.symlink(preprocessSymlinkDestination(target, type, path), pathModule.toNamespacedPath(path), stringToSymlinkType(type), kUsePromises));

          case 4:
          case "end":
            return _context22.stop();
        }
      }
    }, _callee21);
  }));
  return _symlink.apply(this, arguments);
}

function fstat(_x49) {
  return _fstat.apply(this, arguments);
}

function _fstat() {
  _fstat = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee22(handle) {
    var options,
        result,
        _args23 = arguments;
    return regeneratorRuntime.wrap(function _callee22$(_context23) {
      while (1) {
        switch (_context23.prev = _context23.next) {
          case 0:
            options = _args23.length > 1 && _args23[1] !== undefined ? _args23[1] : {
              bigint: false
            };
            _context23.next = 3;
            return binding.fstat(handle.fd, options.bigint, kUsePromises);

          case 3:
            result = _context23.sent;
            return _context23.abrupt("return", getStatsFromBinding(result));

          case 5:
          case "end":
            return _context23.stop();
        }
      }
    }, _callee22);
  }));
  return _fstat.apply(this, arguments);
}

function lstat(_x50) {
  return _lstat.apply(this, arguments);
}

function _lstat() {
  _lstat = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee23(path) {
    var options,
        result,
        _args24 = arguments;
    return regeneratorRuntime.wrap(function _callee23$(_context24) {
      while (1) {
        switch (_context24.prev = _context24.next) {
          case 0:
            options = _args24.length > 1 && _args24[1] !== undefined ? _args24[1] : {
              bigint: false
            };
            path = getValidatedPath(path);
            _context24.next = 4;
            return binding.lstat(pathModule.toNamespacedPath(path), options.bigint, kUsePromises);

          case 4:
            result = _context24.sent;
            return _context24.abrupt("return", getStatsFromBinding(result));

          case 6:
          case "end":
            return _context24.stop();
        }
      }
    }, _callee23);
  }));
  return _lstat.apply(this, arguments);
}

function stat(_x51) {
  return _stat.apply(this, arguments);
}

function _stat() {
  _stat = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee24(path) {
    var options,
        result,
        _args25 = arguments;
    return regeneratorRuntime.wrap(function _callee24$(_context25) {
      while (1) {
        switch (_context25.prev = _context25.next) {
          case 0:
            options = _args25.length > 1 && _args25[1] !== undefined ? _args25[1] : {
              bigint: false
            };
            path = getValidatedPath(path);
            _context25.next = 4;
            return binding.stat(pathModule.toNamespacedPath(path), options.bigint, kUsePromises);

          case 4:
            result = _context25.sent;
            return _context25.abrupt("return", getStatsFromBinding(result));

          case 6:
          case "end":
            return _context25.stop();
        }
      }
    }, _callee24);
  }));
  return _stat.apply(this, arguments);
}

function link(_x52, _x53) {
  return _link.apply(this, arguments);
}

function _link() {
  _link = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee25(existingPath, newPath) {
    return regeneratorRuntime.wrap(function _callee25$(_context26) {
      while (1) {
        switch (_context26.prev = _context26.next) {
          case 0:
            existingPath = getValidatedPath(existingPath, 'existingPath');
            newPath = getValidatedPath(newPath, 'newPath');
            return _context26.abrupt("return", binding.link(pathModule.toNamespacedPath(existingPath), pathModule.toNamespacedPath(newPath), kUsePromises));

          case 3:
          case "end":
            return _context26.stop();
        }
      }
    }, _callee25);
  }));
  return _link.apply(this, arguments);
}

function unlink(_x54) {
  return _unlink.apply(this, arguments);
}

function _unlink() {
  _unlink = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee26(path) {
    return regeneratorRuntime.wrap(function _callee26$(_context27) {
      while (1) {
        switch (_context27.prev = _context27.next) {
          case 0:
            path = getValidatedPath(path);
            return _context27.abrupt("return", binding.unlink(pathModule.toNamespacedPath(path), kUsePromises));

          case 2:
          case "end":
            return _context27.stop();
        }
      }
    }, _callee26);
  }));
  return _unlink.apply(this, arguments);
}

function fchmod(_x55, _x56) {
  return _fchmod.apply(this, arguments);
}

function _fchmod() {
  _fchmod = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee27(handle, mode) {
    return regeneratorRuntime.wrap(function _callee27$(_context28) {
      while (1) {
        switch (_context28.prev = _context28.next) {
          case 0:
            mode = parseFileMode(mode, 'mode');
            return _context28.abrupt("return", binding.fchmod(handle.fd, mode, kUsePromises));

          case 2:
          case "end":
            return _context28.stop();
        }
      }
    }, _callee27);
  }));
  return _fchmod.apply(this, arguments);
}

function chmod(_x57, _x58) {
  return _chmod.apply(this, arguments);
}

function _chmod() {
  _chmod = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee28(path, mode) {
    return regeneratorRuntime.wrap(function _callee28$(_context29) {
      while (1) {
        switch (_context29.prev = _context29.next) {
          case 0:
            path = getValidatedPath(path);
            mode = parseFileMode(mode, 'mode');
            return _context29.abrupt("return", binding.chmod(pathModule.toNamespacedPath(path), mode, kUsePromises));

          case 3:
          case "end":
            return _context29.stop();
        }
      }
    }, _callee28);
  }));
  return _chmod.apply(this, arguments);
}

function lchmod(_x59, _x60) {
  return _lchmod.apply(this, arguments);
}

function _lchmod() {
  _lchmod = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee29(path, mode) {
    var fd;
    return regeneratorRuntime.wrap(function _callee29$(_context30) {
      while (1) {
        switch (_context30.prev = _context30.next) {
          case 0:
            if (!(O_SYMLINK === undefined)) {
              _context30.next = 2;
              break;
            }

            throw new ERR_METHOD_NOT_IMPLEMENTED('lchmod()');

          case 2:
            _context30.next = 4;
            return open(path, O_WRONLY | O_SYMLINK);

          case 4:
            fd = _context30.sent;
            return _context30.abrupt("return", handleFdClose(fchmod(fd, mode), fd.close));

          case 6:
          case "end":
            return _context30.stop();
        }
      }
    }, _callee29);
  }));
  return _lchmod.apply(this, arguments);
}

function lchown(_x61, _x62, _x63) {
  return _lchown.apply(this, arguments);
}

function _lchown() {
  _lchown = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee30(path, uid, gid) {
    return regeneratorRuntime.wrap(function _callee30$(_context31) {
      while (1) {
        switch (_context31.prev = _context31.next) {
          case 0:
            path = getValidatedPath(path);
            validateInteger(uid, 'uid', -1, kMaxUserId);
            validateInteger(gid, 'gid', -1, kMaxUserId);
            return _context31.abrupt("return", binding.lchown(pathModule.toNamespacedPath(path), uid, gid, kUsePromises));

          case 4:
          case "end":
            return _context31.stop();
        }
      }
    }, _callee30);
  }));
  return _lchown.apply(this, arguments);
}

function fchown(_x64, _x65, _x66) {
  return _fchown.apply(this, arguments);
}

function _fchown() {
  _fchown = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee31(handle, uid, gid) {
    return regeneratorRuntime.wrap(function _callee31$(_context32) {
      while (1) {
        switch (_context32.prev = _context32.next) {
          case 0:
            validateInteger(uid, 'uid', -1, kMaxUserId);
            validateInteger(gid, 'gid', -1, kMaxUserId);
            return _context32.abrupt("return", binding.fchown(handle.fd, uid, gid, kUsePromises));

          case 3:
          case "end":
            return _context32.stop();
        }
      }
    }, _callee31);
  }));
  return _fchown.apply(this, arguments);
}

function chown(_x67, _x68, _x69) {
  return _chown.apply(this, arguments);
}

function _chown() {
  _chown = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee32(path, uid, gid) {
    return regeneratorRuntime.wrap(function _callee32$(_context33) {
      while (1) {
        switch (_context33.prev = _context33.next) {
          case 0:
            path = getValidatedPath(path);
            validateInteger(uid, 'uid', -1, kMaxUserId);
            validateInteger(gid, 'gid', -1, kMaxUserId);
            return _context33.abrupt("return", binding.chown(pathModule.toNamespacedPath(path), uid, gid, kUsePromises));

          case 4:
          case "end":
            return _context33.stop();
        }
      }
    }, _callee32);
  }));
  return _chown.apply(this, arguments);
}

function utimes(_x70, _x71, _x72) {
  return _utimes.apply(this, arguments);
}

function _utimes() {
  _utimes = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee33(path, atime, mtime) {
    return regeneratorRuntime.wrap(function _callee33$(_context34) {
      while (1) {
        switch (_context34.prev = _context34.next) {
          case 0:
            path = getValidatedPath(path);
            return _context34.abrupt("return", binding.utimes(pathModule.toNamespacedPath(path), toUnixTimestamp(atime), toUnixTimestamp(mtime), kUsePromises));

          case 2:
          case "end":
            return _context34.stop();
        }
      }
    }, _callee33);
  }));
  return _utimes.apply(this, arguments);
}

function futimes(_x73, _x74, _x75) {
  return _futimes.apply(this, arguments);
}

function _futimes() {
  _futimes = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee34(handle, atime, mtime) {
    return regeneratorRuntime.wrap(function _callee34$(_context35) {
      while (1) {
        switch (_context35.prev = _context35.next) {
          case 0:
            atime = toUnixTimestamp(atime, 'atime');
            mtime = toUnixTimestamp(mtime, 'mtime');
            return _context35.abrupt("return", binding.futimes(handle.fd, atime, mtime, kUsePromises));

          case 3:
          case "end":
            return _context35.stop();
        }
      }
    }, _callee34);
  }));
  return _futimes.apply(this, arguments);
}

function lutimes(_x76, _x77, _x78) {
  return _lutimes.apply(this, arguments);
}

function _lutimes() {
  _lutimes = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee35(path, atime, mtime) {
    return regeneratorRuntime.wrap(function _callee35$(_context36) {
      while (1) {
        switch (_context36.prev = _context36.next) {
          case 0:
            path = getValidatedPath(path);
            return _context36.abrupt("return", binding.lutimes(pathModule.toNamespacedPath(path), toUnixTimestamp(atime), toUnixTimestamp(mtime), kUsePromises));

          case 2:
          case "end":
            return _context36.stop();
        }
      }
    }, _callee35);
  }));
  return _lutimes.apply(this, arguments);
}

function realpath(_x79, _x80) {
  return _realpath.apply(this, arguments);
}

function _realpath() {
  _realpath = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee36(path, options) {
    return regeneratorRuntime.wrap(function _callee36$(_context37) {
      while (1) {
        switch (_context37.prev = _context37.next) {
          case 0:
            options = getOptions(options, {});
            path = getValidatedPath(path);
            return _context37.abrupt("return", binding.realpath(path, options.encoding, kUsePromises));

          case 3:
          case "end":
            return _context37.stop();
        }
      }
    }, _callee36);
  }));
  return _realpath.apply(this, arguments);
}

function mkdtemp(_x81, _x82) {
  return _mkdtemp.apply(this, arguments);
}

function _mkdtemp() {
  _mkdtemp = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee37(prefix, options) {
    return regeneratorRuntime.wrap(function _callee37$(_context38) {
      while (1) {
        switch (_context38.prev = _context38.next) {
          case 0:
            options = getOptions(options, {});

            if (!(!prefix || typeof prefix !== 'string')) {
              _context38.next = 3;
              break;
            }

            throw new ERR_INVALID_ARG_TYPE('prefix', 'string', prefix);

          case 3:
            nullCheck(prefix);
            warnOnNonPortableTemplate(prefix);
            return _context38.abrupt("return", binding.mkdtemp("".concat(prefix, "XXXXXX"), options.encoding, kUsePromises));

          case 6:
          case "end":
            return _context38.stop();
        }
      }
    }, _callee37);
  }));
  return _mkdtemp.apply(this, arguments);
}

function _writeFile(_x83, _x84, _x85) {
  return _writeFile2.apply(this, arguments);
}

function _writeFile2() {
  _writeFile2 = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee38(path, data, options) {
    var flag, fd;
    return regeneratorRuntime.wrap(function _callee38$(_context39) {
      while (1) {
        switch (_context39.prev = _context39.next) {
          case 0:
            options = getOptions(options, {
              encoding: 'utf8',
              mode: 438,
              flag: 'w'
            });
            flag = options.flag || 'w';

            if (!isArrayBufferView(data) && !isCustomIterable(data)) {
              validateStringAfterArrayBufferView(data, 'data');
              data = Buffer.from(data, options.encoding || 'utf8');
            }

            validateAbortSignal(options.signal);

            if (!(path instanceof FileHandle)) {
              _context39.next = 6;
              break;
            }

            return _context39.abrupt("return", writeFileHandle(path, data, options.signal, options.encoding));

          case 6:
            checkAborted(options.signal);
            _context39.next = 9;
            return open(path, flag, options.mode);

          case 9:
            fd = _context39.sent;
            return _context39.abrupt("return", handleFdClose(writeFileHandle(fd, data, options.signal, options.encoding), fd.close));

          case 11:
          case "end":
            return _context39.stop();
        }
      }
    }, _callee38);
  }));
  return _writeFile2.apply(this, arguments);
}

function isCustomIterable(obj) {
  return isIterable(obj) && !isArrayBufferView(obj) && typeof obj !== 'string';
}

function appendFile(_x86, _x87, _x88) {
  return _appendFile.apply(this, arguments);
}

function _appendFile() {
  _appendFile = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee39(path, data, options) {
    return regeneratorRuntime.wrap(function _callee39$(_context40) {
      while (1) {
        switch (_context40.prev = _context40.next) {
          case 0:
            options = getOptions(options, {
              encoding: 'utf8',
              mode: 438,
              flag: 'a'
            });
            options = copyObject(options);
            options.flag = options.flag || 'a';
            return _context40.abrupt("return", _writeFile(path, data, options));

          case 4:
          case "end":
            return _context40.stop();
        }
      }
    }, _callee39);
  }));
  return _appendFile.apply(this, arguments);
}

function _readFile(_x89, _x90) {
  return _readFile2.apply(this, arguments);
}

function _readFile2() {
  _readFile2 = _asyncToGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee40(path, options) {
    var flag, fd;
    return regeneratorRuntime.wrap(function _callee40$(_context41) {
      while (1) {
        switch (_context41.prev = _context41.next) {
          case 0:
            options = getOptions(options, {
              flag: 'r'
            });
            flag = options.flag || 'r';

            if (!(path instanceof FileHandle)) {
              _context41.next = 4;
              break;
            }

            return _context41.abrupt("return", readFileHandle(path, options));

          case 4:
            checkAborted(options.signal);
            _context41.next = 7;
            return open(path, flag, 438);

          case 7:
            fd = _context41.sent;
            return _context41.abrupt("return", handleFdClose(readFileHandle(fd, options), fd.close));

          case 9:
          case "end":
            return _context41.stop();
        }
      }
    }, _callee40);
  }));
  return _readFile2.apply(this, arguments);
}

module.exports = {
  exports: {
    access: access,
    copyFile: copyFile,
    open: open,
    opendir: promisify(opendir),
    rename: rename,
    truncate: truncate,
    rm: rm,
    rmdir: rmdir,
    mkdir: mkdir,
    readdir: readdir,
    readlink: readlink,
    symlink: symlink,
    lstat: lstat,
    stat: stat,
    link: link,
    unlink: unlink,
    chmod: chmod,
    lchmod: lchmod,
    lchown: lchown,
    chown: chown,
    utimes: utimes,
    lutimes: lutimes,
    realpath: realpath,
    mkdtemp: mkdtemp,
    writeFile: _writeFile,
    appendFile: appendFile,
    readFile: _readFile,
    watch: watch
  },
  FileHandle: FileHandle,
  kRef: kRef,
  kUnref: kUnref
};
