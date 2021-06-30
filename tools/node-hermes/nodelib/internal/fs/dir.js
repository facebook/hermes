// @nolint
'use strict';

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _awaitAsyncGenerator(value) { return new _AwaitValue(value); }

function _wrapAsyncGenerator(fn) { return function () { return new _AsyncGenerator(fn.apply(this, arguments)); }; }

function _AsyncGenerator(gen) { var front, back; function send(key, arg) { return new Promise(function (resolve, reject) { var request = { key: key, arg: arg, resolve: resolve, reject: reject, next: null }; if (back) { back = back.next = request; } else { front = back = request; resume(key, arg); } }); } function resume(key, arg) { try { var result = gen[key](arg); var value = result.value; var wrappedAwait = value instanceof _AwaitValue; Promise.resolve(wrappedAwait ? value.wrapped : value).then(function (arg) { if (wrappedAwait) { resume(key === "return" ? "return" : "next", arg); return; } settle(result.done ? "return" : "normal", arg); }, function (err) { resume("throw", err); }); } catch (err) { settle("throw", err); } } function settle(type, value) { switch (type) { case "return": front.resolve({ value: value, done: true }); break; case "throw": front.reject(value); break; default: front.resolve({ value: value, done: false }); break; } front = front.next; if (front) { resume(front.key, front.arg); } else { back = null; } } this._invoke = send; if (typeof gen["return"] !== "function") { this["return"] = undefined; } }

_AsyncGenerator.prototype[typeof Symbol === "function" && Symbol.asyncIterator || "@@asyncIterator"] = function () { return this; };

_AsyncGenerator.prototype.next = function (arg) { return this._invoke("next", arg); };

_AsyncGenerator.prototype["throw"] = function (arg) { return this._invoke("throw", arg); };

_AsyncGenerator.prototype["return"] = function (arg) { return this._invoke("return", arg); };

function _AwaitValue(value) { this.wrapped = value; }

var _primordials = primordials,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    ArrayPrototypeSlice = _primordials.ArrayPrototypeSlice,
    ArrayPrototypeSplice = _primordials.ArrayPrototypeSplice,
    FunctionPrototypeBind = _primordials.FunctionPrototypeBind,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    PromiseReject = _primordials.PromiseReject,
    _Symbol = _primordials.Symbol,
    SymbolAsyncIterator = _primordials.SymbolAsyncIterator;

var pathModule = require('path');

var binding = internalBinding('fs');
var dirBinding = internalBinding('fs_dir');

var _require = require('internal/errors'),
    _require$codes = _require.codes,
    ERR_DIR_CLOSED = _require$codes.ERR_DIR_CLOSED,
    ERR_DIR_CONCURRENT_OPERATION = _require$codes.ERR_DIR_CONCURRENT_OPERATION,
    ERR_MISSING_ARGS = _require$codes.ERR_MISSING_ARGS;

var FSReqCallback = binding.FSReqCallback;

var internalUtil = require('internal/util');

var _require2 = require('internal/fs/utils'),
    getDirent = _require2.getDirent,
    getOptions = _require2.getOptions,
    getValidatedPath = _require2.getValidatedPath,
    handleErrorFromBinding = _require2.handleErrorFromBinding;

var _require3 = require('internal/validators'),
    validateCallback = _require3.validateCallback,
    validateUint32 = _require3.validateUint32;

var kDirHandle = _Symbol('kDirHandle');

var kDirPath = _Symbol('kDirPath');

var kDirBufferedEntries = _Symbol('kDirBufferedEntries');

var kDirClosed = _Symbol('kDirClosed');

var kDirOptions = _Symbol('kDirOptions');

var kDirReadImpl = _Symbol('kDirReadImpl');

var kDirReadPromisified = _Symbol('kDirReadPromisified');

var kDirClosePromisified = _Symbol('kDirClosePromisified');

var kDirOperationQueue = _Symbol('kDirOperationQueue');

var Dir = /*#__PURE__*/function () {
  function Dir(handle, path, options) {
    _classCallCheck(this, Dir);

    if (handle == null) throw new ERR_MISSING_ARGS('handle');
    this[kDirHandle] = handle;
    this[kDirBufferedEntries] = [];
    this[kDirPath] = path;
    this[kDirClosed] = false; // Either `null` or an Array of pending operations (= functions to be called
    // once the current operation is done).

    this[kDirOperationQueue] = null;
    this[kDirOptions] = _objectSpread({
      bufferSize: 32
    }, getOptions(options, {
      encoding: 'utf8'
    }));
    validateUint32(this[kDirOptions].bufferSize, 'options.bufferSize', true);
    this[kDirReadPromisified] = FunctionPrototypeBind(internalUtil.promisify(this[kDirReadImpl]), this, false);
    this[kDirClosePromisified] = FunctionPrototypeBind(internalUtil.promisify(this.close), this);
  }

  _createClass(Dir, [{
    key: "path",
    get: function get() {
      return this[kDirPath];
    }
  }, {
    key: "read",
    value: function read(callback) {
      return this[kDirReadImpl](true, callback);
    }
  }, {
    key: kDirReadImpl,
    value: function value(maybeSync, callback) {
      var _this2 = this;

      if (this[kDirClosed] === true) {
        throw new ERR_DIR_CLOSED();
      }

      if (callback === undefined) {
        return this[kDirReadPromisified]();
      }

      validateCallback(callback);

      if (this[kDirOperationQueue] !== null) {
        ArrayPrototypePush(this[kDirOperationQueue], function () {
          _this2[kDirReadImpl](maybeSync, callback);
        });
        return;
      }

      if (this[kDirBufferedEntries].length > 0) {
        var _ArrayPrototypeSplice = ArrayPrototypeSplice(this[kDirBufferedEntries], 0, 2),
            name = _ArrayPrototypeSplice[0],
            type = _ArrayPrototypeSplice[1];

        if (maybeSync) process.nextTick(getDirent, this[kDirPath], name, type, callback);else getDirent(this[kDirPath], name, type, callback);
        return;
      }

      var req = new FSReqCallback();

      req.oncomplete = function (err, result) {
        process.nextTick(function () {
          var queue = _this2[kDirOperationQueue];
          _this2[kDirOperationQueue] = null;

          var _iterator = _createForOfIteratorHelper(queue),
              _step;

          try {
            for (_iterator.s(); !(_step = _iterator.n()).done;) {
              var op = _step.value;
              op();
            }
          } catch (err) {
            _iterator.e(err);
          } finally {
            _iterator.f();
          }
        });

        if (err || result === null) {
          return callback(err, result);
        }

        _this2[kDirBufferedEntries] = ArrayPrototypeSlice(result, 2);
        getDirent(_this2[kDirPath], result[0], result[1], callback);
      };

      this[kDirOperationQueue] = [];
      this[kDirHandle].read(this[kDirOptions].encoding, this[kDirOptions].bufferSize, req);
    }
  }, {
    key: "readSync",
    value: function readSync() {
      if (this[kDirClosed] === true) {
        throw new ERR_DIR_CLOSED();
      }

      if (this[kDirOperationQueue] !== null) {
        throw new ERR_DIR_CONCURRENT_OPERATION();
      }

      if (this[kDirBufferedEntries].length > 0) {
        var _ArrayPrototypeSplice2 = ArrayPrototypeSplice(this[kDirBufferedEntries], 0, 2),
            name = _ArrayPrototypeSplice2[0],
            type = _ArrayPrototypeSplice2[1];

        return getDirent(this[kDirPath], name, type);
      }

      var ctx = {
        path: this[kDirPath]
      };
      var result = this[kDirHandle].read(this[kDirOptions].encoding, this[kDirOptions].bufferSize, undefined, ctx);
      handleErrorFromBinding(ctx);

      if (result === null) {
        return result;
      }

      this[kDirBufferedEntries] = ArrayPrototypeSlice(result, 2);
      return getDirent(this[kDirPath], result[0], result[1]);
    }
  }, {
    key: "close",
    value: function close(callback) {
      var _this3 = this;

      // Promise
      if (callback === undefined) {
        if (this[kDirClosed] === true) {
          return PromiseReject(new ERR_DIR_CLOSED());
        }

        return this[kDirClosePromisified]();
      } // callback


      validateCallback(callback);

      if (this[kDirClosed] === true) {
        process.nextTick(callback, new ERR_DIR_CLOSED());
        return;
      }

      if (this[kDirOperationQueue] !== null) {
        ArrayPrototypePush(this[kDirOperationQueue], function () {
          _this3.close(callback);
        });
        return;
      }

      this[kDirClosed] = true;
      var req = new FSReqCallback();
      req.oncomplete = callback;
      this[kDirHandle].close(req);
    }
  }, {
    key: "closeSync",
    value: function closeSync() {
      if (this[kDirClosed] === true) {
        throw new ERR_DIR_CLOSED();
      }

      if (this[kDirOperationQueue] !== null) {
        throw new ERR_DIR_CONCURRENT_OPERATION();
      }

      this[kDirClosed] = true;
      var ctx = {
        path: this[kDirPath]
      };
      var result = this[kDirHandle].close(undefined, ctx);
      handleErrorFromBinding(ctx);
      return result;
    }
  }, {
    key: "entries",
    value: function entries() {
      var _this = this;

      return _wrapAsyncGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee() {
        var result;
        return regeneratorRuntime.wrap(function _callee$(_context) {
          while (1) {
            switch (_context.prev = _context.next) {
              case 0:
                _context.prev = 0;

              case 1:
                if (!true) {
                  _context.next = 11;
                  break;
                }

                _context.next = 4;
                return _awaitAsyncGenerator(_this[kDirReadPromisified]());

              case 4:
                result = _context.sent;

                if (!(result === null)) {
                  _context.next = 7;
                  break;
                }

                return _context.abrupt("break", 11);

              case 7:
                _context.next = 9;
                return result;

              case 9:
                _context.next = 1;
                break;

              case 11:
                _context.prev = 11;
                _context.next = 14;
                return _awaitAsyncGenerator(_this[kDirClosePromisified]());

              case 14:
                return _context.finish(11);

              case 15:
              case "end":
                return _context.stop();
            }
          }
        }, _callee, null, [[0,, 11, 15]]);
      }))();
    }
  }]);

  return Dir;
}();

ObjectDefineProperty(Dir.prototype, SymbolAsyncIterator, {
  value: Dir.prototype.entries,
  enumerable: false,
  writable: true,
  configurable: true
});

function opendir(path, options, callback) {
  callback = typeof options === 'function' ? options : callback;
  validateCallback(callback);
  path = getValidatedPath(path);
  options = getOptions(options, {
    encoding: 'utf8'
  });

  function opendirCallback(error, handle) {
    if (error) {
      callback(error);
    } else {
      callback(null, new Dir(handle, path, options));
    }
  }

  var req = new FSReqCallback();
  req.oncomplete = opendirCallback;
  dirBinding.opendir(pathModule.toNamespacedPath(path), options.encoding, req);
}

function opendirSync(path, options) {
  path = getValidatedPath(path);
  options = getOptions(options, {
    encoding: 'utf8'
  });
  var ctx = {
    path: path
  };
  var handle = dirBinding.opendir(pathModule.toNamespacedPath(path), options.encoding, undefined, ctx);
  handleErrorFromBinding(ctx);
  return new Dir(handle, path, options);
}

module.exports = {
  Dir: Dir,
  opendir: opendir,
  opendirSync: opendirSync
};
