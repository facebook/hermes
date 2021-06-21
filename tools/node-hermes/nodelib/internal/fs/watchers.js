// @nolint
'use strict';

function _awaitAsyncGenerator(value) { return new _AwaitValue(value); }

function _wrapAsyncGenerator(fn) { return function () { return new _AsyncGenerator(fn.apply(this, arguments)); }; }

function _AsyncGenerator(gen) { var front, back; function send(key, arg) { return new Promise(function (resolve, reject) { var request = { key: key, arg: arg, resolve: resolve, reject: reject, next: null }; if (back) { back = back.next = request; } else { front = back = request; resume(key, arg); } }); } function resume(key, arg) { try { var result = gen[key](arg); var value = result.value; var wrappedAwait = value instanceof _AwaitValue; Promise.resolve(wrappedAwait ? value.wrapped : value).then(function (arg) { if (wrappedAwait) { resume(key === "return" ? "return" : "next", arg); return; } settle(result.done ? "return" : "normal", arg); }, function (err) { resume("throw", err); }); } catch (err) { settle("throw", err); } } function settle(type, value) { switch (type) { case "return": front.resolve({ value: value, done: true }); break; case "throw": front.reject(value); break; default: front.resolve({ value: value, done: false }); break; } front = front.next; if (front) { resume(front.key, front.arg); } else { back = null; } } this._invoke = send; if (typeof gen["return"] !== "function") { this["return"] = undefined; } }

_AsyncGenerator.prototype[typeof Symbol === "function" && Symbol.asyncIterator || "@@asyncIterator"] = function () { return this; };

_AsyncGenerator.prototype.next = function (arg) { return this._invoke("next", arg); };

_AsyncGenerator.prototype["throw"] = function (arg) { return this._invoke("throw", arg); };

_AsyncGenerator.prototype["return"] = function (arg) { return this._invoke("return", arg); };

function _AwaitValue(value) { this.wrapped = value; }

var _primordials = primordials,
    FunctionPrototypeCall = _primordials.FunctionPrototypeCall,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    _Symbol = _primordials.Symbol;

var _require = require('internal/errors'),
    AbortError = _require.AbortError,
    uvException = _require.uvException,
    ERR_INVALID_ARG_VALUE = _require.codes.ERR_INVALID_ARG_VALUE;

var _require2 = require('internal/util'),
    createDeferredPromise = _require2.createDeferredPromise;

var _internalBinding = internalBinding('fs'),
    kFsStatsFieldsNumber = _internalBinding.kFsStatsFieldsNumber,
    _StatWatcher = _internalBinding.StatWatcher;

var _internalBinding2 = internalBinding('fs_event_wrap'),
    FSEvent = _internalBinding2.FSEvent;

var _internalBinding3 = internalBinding('uv'),
    UV_ENOSPC = _internalBinding3.UV_ENOSPC;

var _require3 = require('events'),
    EventEmitter = _require3.EventEmitter;

var _require4 = require('internal/fs/utils'),
    getStatsFromBinding = _require4.getStatsFromBinding,
    getValidatedPath = _require4.getValidatedPath;

var _require5 = require('internal/async_hooks'),
    defaultTriggerAsyncIdScope = _require5.defaultTriggerAsyncIdScope,
    owner_symbol = _require5.symbols.owner_symbol;

var _require6 = require('path'),
    toNamespacedPath = _require6.toNamespacedPath;

var _require7 = require('internal/validators'),
    validateAbortSignal = _require7.validateAbortSignal,
    validateBoolean = _require7.validateBoolean,
    validateObject = _require7.validateObject,
    validateUint32 = _require7.validateUint32;

var _require8 = require('buffer'),
    isEncoding = _require8.Buffer.isEncoding;

var assert = require('internal/assert');

var kOldStatus = _Symbol('kOldStatus');

var kUseBigint = _Symbol('kUseBigint');

var kFSWatchStart = _Symbol('kFSWatchStart');

var kFSStatWatcherStart = _Symbol('kFSStatWatcherStart');

var KFSStatWatcherRefCount = _Symbol('KFSStatWatcherRefCount');

var KFSStatWatcherMaxRefCount = _Symbol('KFSStatWatcherMaxRefCount');

var kFSStatWatcherAddOrCleanRef = _Symbol('kFSStatWatcherAddOrCleanRef');

function emitStop(self) {
  self.emit('stop');
}

function StatWatcher(bigint) {
  FunctionPrototypeCall(EventEmitter, this);
  this._handle = null;
  this[kOldStatus] = -1;
  this[kUseBigint] = bigint;
  this[KFSStatWatcherRefCount] = 1;
  this[KFSStatWatcherMaxRefCount] = 1;
}

ObjectSetPrototypeOf(StatWatcher.prototype, EventEmitter.prototype);
ObjectSetPrototypeOf(StatWatcher, EventEmitter);

function onchange(newStatus, stats) {
  var self = this[owner_symbol];

  if (self[kOldStatus] === -1 && newStatus === -1 && stats[2
  /* new nlink */
  ] === stats[16
  /* old nlink */
  ]) {
    return;
  }

  self[kOldStatus] = newStatus;
  self.emit('change', getStatsFromBinding(stats), getStatsFromBinding(stats, kFsStatsFieldsNumber));
} // At the moment if filename is undefined, we
// 1. Throw an Error if it's the first
//    time Symbol('kFSStatWatcherStart') is called
// 2. Return silently if Symbol('kFSStatWatcherStart') has already been called
//    on a valid filename and the wrap has been initialized
// This method is a noop if the watcher has already been started.


StatWatcher.prototype[kFSStatWatcherStart] = function (filename, persistent, interval) {
  if (this._handle !== null) return;
  this._handle = new _StatWatcher(this[kUseBigint]);
  this._handle[owner_symbol] = this;
  this._handle.onchange = onchange;
  if (!persistent) this.unref(); // uv_fs_poll is a little more powerful than ev_stat but we curb it for
  // the sake of backwards compatibility.

  this[kOldStatus] = -1;
  filename = getValidatedPath(filename, 'filename');
  validateUint32(interval, 'interval');

  var err = this._handle.start(toNamespacedPath(filename), interval);

  if (err) {
    var error = uvException({
      errno: err,
      syscall: 'watch',
      path: filename
    });
    error.filename = filename;
    throw error;
  }
}; // To maximize backward-compatibility for the end user,
// a no-op stub method has been added instead of
// totally removing StatWatcher.prototype.start.
// This should not be documented.


StatWatcher.prototype.start = function () {}; // FIXME(joyeecheung): this method is not documented while there is
// another documented fs.unwatchFile(). The counterpart in
// FSWatcher is .close()
// This method is a noop if the watcher has not been started.


StatWatcher.prototype.stop = function () {
  if (this._handle === null) return;
  defaultTriggerAsyncIdScope(this._handle.getAsyncId(), process.nextTick, emitStop, this);

  this._handle.close();

  this._handle = null;
}; // Clean up or add ref counters.


StatWatcher.prototype[kFSStatWatcherAddOrCleanRef] = function (operate) {
  if (operate === 'add') {
    // Add a Ref
    this[KFSStatWatcherRefCount]++;
    this[KFSStatWatcherMaxRefCount]++;
  } else if (operate === 'clean') {
    // Clean up a single
    this[KFSStatWatcherMaxRefCount]--;
    this.unref();
  } else if (operate === 'cleanAll') {
    var _this$_handle;

    // Clean up all
    this[KFSStatWatcherMaxRefCount] = 0;
    this[KFSStatWatcherRefCount] = 0;
    (_this$_handle = this._handle) === null || _this$_handle === void 0 ? void 0 : _this$_handle.unref();
  }
};

StatWatcher.prototype.ref = function () {
  // Avoid refCount calling ref multiple times causing unref to have no effect.
  if (this[KFSStatWatcherRefCount] === this[KFSStatWatcherMaxRefCount]) return this;
  if (this._handle && this[KFSStatWatcherRefCount]++ === 0) this._handle.ref();
  return this;
};

StatWatcher.prototype.unref = function () {
  // Avoid refCount calling unref multiple times causing ref to have no effect.
  if (this[KFSStatWatcherRefCount] === 0) return this;
  if (this._handle && --this[KFSStatWatcherRefCount] === 0) this._handle.unref();
  return this;
};

function FSWatcher() {
  var _this = this;

  FunctionPrototypeCall(EventEmitter, this);
  this._handle = new FSEvent();
  this._handle[owner_symbol] = this;

  this._handle.onchange = function (status, eventType, filename) {
    // TODO(joyeecheung): we may check self._handle.initialized here
    // and return if that is false. This allows us to avoid firing the event
    // after the handle is closed, and to fire both UV_RENAME and UV_CHANGE
    // if they are set by libuv at the same time.
    if (status < 0) {
      if (_this._handle !== null) {
        // We don't use this.close() here to avoid firing the close event.
        _this._handle.close();

        _this._handle = null; // Make the handle garbage collectable.
      }

      var error = uvException({
        errno: status,
        syscall: 'watch',
        path: filename
      });
      error.filename = filename;

      _this.emit('error', error);
    } else {
      _this.emit('change', eventType, filename);
    }
  };
}

ObjectSetPrototypeOf(FSWatcher.prototype, EventEmitter.prototype);
ObjectSetPrototypeOf(FSWatcher, EventEmitter); // At the moment if filename is undefined, we
// 1. Throw an Error if it's the first time Symbol('kFSWatchStart') is called
// 2. Return silently if Symbol('kFSWatchStart') has already been called
//    on a valid filename and the wrap has been initialized
// 3. Return silently if the watcher has already been closed
// This method is a noop if the watcher has already been started.

FSWatcher.prototype[kFSWatchStart] = function (filename, persistent, recursive, encoding) {
  if (this._handle === null) {
    // closed
    return;
  }

  assert(this._handle instanceof FSEvent, 'handle must be a FSEvent');

  if (this._handle.initialized) {
    // already started
    return;
  }

  filename = getValidatedPath(filename, 'filename');

  var err = this._handle.start(toNamespacedPath(filename), persistent, recursive, encoding);

  if (err) {
    var error = uvException({
      errno: err,
      syscall: 'watch',
      path: filename,
      message: err === UV_ENOSPC ? 'System limit for number of file watchers reached' : ''
    });
    error.filename = filename;
    throw error;
  }
}; // To maximize backward-compatibility for the end user,
// a no-op stub method has been added instead of
// totally removing FSWatcher.prototype.start.
// This should not be documented.


FSWatcher.prototype.start = function () {}; // This method is a noop if the watcher has not been started or
// has already been closed.


FSWatcher.prototype.close = function () {
  if (this._handle === null) {
    // closed
    return;
  }

  assert(this._handle instanceof FSEvent, 'handle must be a FSEvent');

  if (!this._handle.initialized) {
    // not started
    return;
  }

  this._handle.close();

  this._handle = null; // Make the handle garbage collectable.

  process.nextTick(emitCloseNT, this);
};

FSWatcher.prototype.ref = function () {
  if (this._handle) this._handle.ref();
  return this;
};

FSWatcher.prototype.unref = function () {
  if (this._handle) this._handle.unref();
  return this;
};

function emitCloseNT(self) {
  self.emit('close');
} // Legacy alias on the C++ wrapper object. This is not public API, so we may
// want to runtime-deprecate it at some point. There's no hurry, though.


ObjectDefineProperty(FSEvent.prototype, 'owner', {
  get: function get() {
    return this[owner_symbol];
  },
  set: function set(v) {
    return this[owner_symbol] = v;
  }
});

function watch(_x) {
  return _watch.apply(this, arguments);
}

function _watch() {
  _watch = _wrapAsyncGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee(filename) {
    var options,
        path,
        _options$persistent,
        persistent,
        _options$recursive,
        recursive,
        _options$encoding,
        encoding,
        signal,
        reason,
        handle,
        _createDeferredPromis,
        promise,
        resolve,
        reject,
        oncancel,
        err,
        error,
        _createDeferredPromis2,
        _args = arguments;

    return regeneratorRuntime.wrap(function _callee$(_context) {
      while (1) {
        switch (_context.prev = _context.next) {
          case 0:
            options = _args.length > 1 && _args[1] !== undefined ? _args[1] : {};
            path = toNamespacedPath(getValidatedPath(filename));
            validateObject(options, 'options');
            _options$persistent = options.persistent, persistent = _options$persistent === void 0 ? true : _options$persistent, _options$recursive = options.recursive, recursive = _options$recursive === void 0 ? false : _options$recursive, _options$encoding = options.encoding, encoding = _options$encoding === void 0 ? 'utf8' : _options$encoding, signal = options.signal;
            validateBoolean(persistent, 'options.persistent');
            validateBoolean(recursive, 'options.recursive');
            validateAbortSignal(signal, 'options.signal');

            if (!(encoding && !isEncoding(encoding))) {
              _context.next = 10;
              break;
            }

            reason = 'is invalid encoding';
            throw new ERR_INVALID_ARG_VALUE(encoding, 'encoding', reason);

          case 10:
            if (!(signal !== null && signal !== void 0 && signal.aborted)) {
              _context.next = 12;
              break;
            }

            throw new AbortError();

          case 12:
            handle = new FSEvent();
            _createDeferredPromis = createDeferredPromise(), promise = _createDeferredPromis.promise, resolve = _createDeferredPromis.resolve, reject = _createDeferredPromis.reject;

            oncancel = function oncancel() {
              handle.close();
              reject(new AbortError());
            };

            _context.prev = 15;
            signal === null || signal === void 0 ? void 0 : signal.addEventListener('abort', oncancel, {
              once: true
            });

            handle.onchange = function (status, eventType, filename) {
              if (status < 0) {
                var error = uvException({
                  errno: status,
                  syscall: 'watch',
                  path: filename
                });
                error.filename = filename;
                handle.close();
                reject(error);
                return;
              }

              resolve({
                eventType: eventType,
                filename: filename
              });
            };

            err = handle.start(path, persistent, recursive, encoding);

            if (!err) {
              _context.next = 24;
              break;
            }

            error = uvException({
              errno: err,
              syscall: 'watch',
              path: filename,
              message: err === UV_ENOSPC ? 'System limit for number of file watchers reached' : ''
            });
            error.filename = filename;
            handle.close();
            throw error;

          case 24:
            if (signal !== null && signal !== void 0 && signal.aborted) {
              _context.next = 33;
              break;
            }

            _context.next = 27;
            return promise;

          case 27:
            _createDeferredPromis2 = createDeferredPromise();
            promise = _createDeferredPromis2.promise;
            resolve = _createDeferredPromis2.resolve;
            reject = _createDeferredPromis2.reject;
            _context.next = 24;
            break;

          case 33:
            throw new AbortError();

          case 34:
            _context.prev = 34;
            handle.close();
            signal === null || signal === void 0 ? void 0 : signal.removeEventListener('abort', oncancel);
            return _context.finish(34);

          case 38:
          case "end":
            return _context.stop();
        }
      }
    }, _callee, null, [[15,, 34, 38]]);
  }));
  return _watch.apply(this, arguments);
}

module.exports = {
  FSWatcher: FSWatcher,
  StatWatcher: StatWatcher,
  kFSWatchStart: kFSWatchStart,
  kFSStatWatcherStart: kFSStatWatcherStart,
  kFSStatWatcherAddOrCleanRef: kFSStatWatcherAddOrCleanRef,
  watch: watch
};
