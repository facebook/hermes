// @nolint
'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var _primordials = primordials,
    Array = _primordials.Array,
    FunctionPrototypeBind = _primordials.FunctionPrototypeBind,
    MathMin = _primordials.MathMin,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    PromisePrototypeThen = _primordials.PromisePrototypeThen,
    ReflectApply = _primordials.ReflectApply,
    _Symbol = _primordials.Symbol;

var _require$codes = require('internal/errors').codes,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_OUT_OF_RANGE = _require$codes.ERR_OUT_OF_RANGE,
    ERR_METHOD_NOT_IMPLEMENTED = _require$codes.ERR_METHOD_NOT_IMPLEMENTED;

var _require = require('internal/util'),
    deprecate = _require.deprecate;

var _require2 = require('internal/validators'),
    validateFunction = _require2.validateFunction,
    validateInteger = _require2.validateInteger;

var _require3 = require('internal/streams/destroy'),
    errorOrDestroy = _require3.errorOrDestroy;

var fs = require('fs');

var _require4 = require('internal/fs/promises'),
    kRef = _require4.kRef,
    kUnref = _require4.kUnref,
    FileHandle = _require4.FileHandle;

var _require5 = require('buffer'),
    Buffer = _require5.Buffer;

var _require6 = require('internal/fs/utils'),
    copyObject = _require6.copyObject,
    getOptions = _require6.getOptions,
    getValidatedFd = _require6.getValidatedFd,
    validatePath = _require6.validatePath;

var _require7 = require('stream'),
    Readable = _require7.Readable,
    Writable = _require7.Writable,
    finished = _require7.finished;

var _require8 = require('internal/url'),
    toPathIfFileURL = _require8.toPathIfFileURL;

var kIoDone = _Symbol('kIoDone');

var kIsPerformingIO = _Symbol('kIsPerformingIO');

var kFs = _Symbol('kFs');

var kHandle = _Symbol('kHandle');

function _construct(callback) {
  var stream = this;

  if (typeof stream.fd === 'number') {
    callback();
    return;
  }

  if (stream.open !== openWriteFs && stream.open !== openReadFs) {
    // Backwards compat for monkey patching open().
    var orgEmit = stream.emit;

    stream.emit = function () {
      for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
        args[_key] = arguments[_key];
      }

      if (args[0] === 'open') {
        this.emit = orgEmit;
        callback();
        ReflectApply(orgEmit, this, args);
      } else if (args[0] === 'error') {
        this.emit = orgEmit;
        callback(args[1]);
      } else {
        ReflectApply(orgEmit, this, args);
      }
    };

    stream.open();
  } else {
    stream[kFs].open(stream.path, stream.flags, stream.mode, function (er, fd) {
      if (er) {
        callback(er);
      } else {
        stream.fd = fd;
        callback();
        stream.emit('open', stream.fd);
        stream.emit('ready');
      }
    });
  }
} // This generates an fs operations structure for a FileHandle


var FileHandleOperations = function FileHandleOperations(handle) {
  return {
    open: function open(path, flags, mode, cb) {
      throw new ERR_METHOD_NOT_IMPLEMENTED('open()');
    },
    close: function close(fd, cb) {
      handle[kUnref]();
      PromisePrototypeThen(handle.close(), function () {
        return cb();
      }, cb);
    },
    read: function read(fd, buf, offset, length, pos, cb) {
      PromisePrototypeThen(handle.read(buf, offset, length, pos), function (r) {
        return cb(null, r.bytesRead, r.buffer);
      }, function (err) {
        return cb(err, 0, buf);
      });
    },
    write: function write(fd, buf, offset, length, pos, cb) {
      PromisePrototypeThen(handle.write(buf, offset, length, pos), function (r) {
        return cb(null, r.bytesWritten, r.buffer);
      }, function (err) {
        return cb(err, 0, buf);
      });
    },
    writev: function writev(fd, buffers, pos, cb) {
      PromisePrototypeThen(handle.writev(buffers, pos), function (r) {
        return cb(null, r.bytesWritten, r.buffers);
      }, function (err) {
        return cb(err, 0, buffers);
      });
    }
  };
};

function close(stream, err, cb) {
  if (!stream.fd) {
    // TODO(ronag)
    // stream.closed = true;
    cb(err);
  } else {
    stream[kFs].close(stream.fd, function (er) {
      stream.closed = true;
      cb(er || err);
    });
    stream.fd = null;
  }
}

function importFd(stream, options) {
  stream.fd = null;

  if (options.fd != null) {
    if (typeof options.fd === 'number') {
      // When fd is a raw descriptor, we must keep our fingers crossed
      // that the descriptor won't get closed, or worse, replaced with
      // another one
      // https://github.com/nodejs/node/issues/35862
      stream.fd = options.fd;
    } else if (_typeof(options.fd) === 'object' && options.fd instanceof FileHandle) {
      // When fd is a FileHandle we can listen for 'close' events
      if (options.fs) // FileHandle is not supported with custom fs operations
        throw new ERR_METHOD_NOT_IMPLEMENTED('FileHandle with fs');
      stream[kHandle] = options.fd;
      stream.fd = options.fd.fd;
      stream[kFs] = FileHandleOperations(stream[kHandle]);
      stream[kHandle][kRef]();
      options.fd.on('close', FunctionPrototypeBind(stream.close, stream));
    } else throw ERR_INVALID_ARG_TYPE('options.fd', ['number', 'FileHandle'], options.fd);
  }
}

function ReadStream(path, options) {
  if (!(this instanceof ReadStream)) return new ReadStream(path, options); // A little bit bigger buffer and water marks by default

  options = copyObject(getOptions(options, {}));
  if (options.highWaterMark === undefined) options.highWaterMark = 64 * 1024;

  if (options.autoDestroy === undefined) {
    options.autoDestroy = false;
  }

  this[kFs] = options.fs || fs;
  validateFunction(this[kFs].open, 'options.fs.open');
  validateFunction(this[kFs].read, 'options.fs.read');
  validateFunction(this[kFs].close, 'options.fs.close');
  options.autoDestroy = options.autoClose === undefined ? true : options.autoClose; // Path will be ignored when fd is specified, so it can be falsy

  this.path = toPathIfFileURL(path);
  this.flags = options.flags === undefined ? 'r' : options.flags;
  this.mode = options.mode === undefined ? 438 : options.mode;
  importFd(this, options);
  this.start = options.start;
  this.end = options.end;
  this.pos = undefined;
  this.bytesRead = 0;
  this.closed = false;
  this[kIsPerformingIO] = false;

  if (this.start !== undefined) {
    validateInteger(this.start, 'start', 0);
    this.pos = this.start;
  } // If fd has been set, validate, otherwise validate path.


  if (this.fd != null) {
    this.fd = getValidatedFd(this.fd);
  } else {
    validatePath(this.path);
  }

  if (this.end === undefined) {
    this.end = Infinity;
  } else if (this.end !== Infinity) {
    validateInteger(this.end, 'end', 0);

    if (this.start !== undefined && this.start > this.end) {
      throw new ERR_OUT_OF_RANGE('start', "<= \"end\" (here: ".concat(this.end, ")"), this.start);
    }
  }

  ReflectApply(Readable, this, [options]);
}

ObjectSetPrototypeOf(ReadStream.prototype, Readable.prototype);
ObjectSetPrototypeOf(ReadStream, Readable);
ObjectDefineProperty(ReadStream.prototype, 'autoClose', {
  get: function get() {
    return this._readableState.autoDestroy;
  },
  set: function set(val) {
    this._readableState.autoDestroy = val;
  }
});
var openReadFs = deprecate(function () {// Noop.
}, 'ReadStream.prototype.open() is deprecated', 'DEP0135');
ReadStream.prototype.open = openReadFs;
ReadStream.prototype._construct = _construct;

ReadStream.prototype._read = function (n) {
  var _this = this;

  n = this.pos !== undefined ? MathMin(this.end - this.pos + 1, n) : MathMin(this.end - this.bytesRead + 1, n);

  if (n <= 0) {
    this.push(null);
    return;
  }

  var buf = Buffer.allocUnsafeSlow(n);
  this[kIsPerformingIO] = true;
  this[kFs].read(this.fd, buf, 0, n, this.pos, function (er, bytesRead, buf) {
    _this[kIsPerformingIO] = false; // Tell ._destroy() that it's safe to close the fd now.

    if (_this.destroyed) {
      _this.emit(kIoDone, er);

      return;
    }

    if (er) {
      errorOrDestroy(_this, er);
    } else if (bytesRead > 0) {
      if (_this.pos !== undefined) {
        _this.pos += bytesRead;
      }

      _this.bytesRead += bytesRead;

      if (bytesRead !== buf.length) {
        // Slow path. Shrink to fit.
        // Copy instead of slice so that we don't retain
        // large backing buffer for small reads.
        var dst = Buffer.allocUnsafeSlow(bytesRead);
        buf.copy(dst, 0, 0, bytesRead);
        buf = dst;
      }

      _this.push(buf);
    } else {
      _this.push(null);
    }
  });
};

ReadStream.prototype._destroy = function (err, cb) {
  var _this2 = this;

  // Usually for async IO it is safe to close a file descriptor
  // even when there are pending operations. However, due to platform
  // differences file IO is implemented using synchronous operations
  // running in a thread pool. Therefore, file descriptors are not safe
  // to close while used in a pending read or write operation. Wait for
  // any pending IO (kIsPerformingIO) to complete (kIoDone).
  if (this[kIsPerformingIO]) {
    this.once(kIoDone, function (er) {
      return close(_this2, err || er, cb);
    });
  } else {
    close(this, err, cb);
  }
};

ReadStream.prototype.close = function (cb) {
  if (typeof cb === 'function') finished(this, cb);
  this.destroy();
};

ObjectDefineProperty(ReadStream.prototype, 'pending', {
  get: function get() {
    return this.fd === null;
  },
  configurable: true
});

function WriteStream(path, options) {
  if (!(this instanceof WriteStream)) return new WriteStream(path, options);
  options = copyObject(getOptions(options, {})); // Only buffers are supported.

  options.decodeStrings = true;
  this[kFs] = options.fs || fs;
  validateFunction(this[kFs].open, 'options.fs.open');

  if (!this[kFs].write && !this[kFs].writev) {
    throw new ERR_INVALID_ARG_TYPE('options.fs.write', 'function', this[kFs].write);
  }

  if (this[kFs].write) {
    validateFunction(this[kFs].write, 'options.fs.write');
  }

  if (this[kFs].writev) {
    validateFunction(this[kFs].writev, 'options.fs.writev');
  }

  validateFunction(this[kFs].close, 'options.fs.close'); // It's enough to override either, in which case only one will be used.

  if (!this[kFs].write) {
    this._write = null;
  }

  if (!this[kFs].writev) {
    this._writev = null;
  }

  options.autoDestroy = options.autoClose === undefined ? true : options.autoClose; // Path will be ignored when fd is specified, so it can be falsy

  this.path = toPathIfFileURL(path);
  this.flags = options.flags === undefined ? 'w' : options.flags;
  this.mode = options.mode === undefined ? 438 : options.mode;
  importFd(this, options);
  this.start = options.start;
  this.pos = undefined;
  this.bytesWritten = 0;
  this.closed = false;
  this[kIsPerformingIO] = false; // If fd has been set, validate, otherwise validate path.

  if (this.fd != null) {
    this.fd = getValidatedFd(this.fd);
  } else {
    validatePath(this.path);
  }

  if (this.start !== undefined) {
    validateInteger(this.start, 'start', 0);
    this.pos = this.start;
  }

  ReflectApply(Writable, this, [options]);
  if (options.encoding) this.setDefaultEncoding(options.encoding);
}

ObjectSetPrototypeOf(WriteStream.prototype, Writable.prototype);
ObjectSetPrototypeOf(WriteStream, Writable);
ObjectDefineProperty(WriteStream.prototype, 'autoClose', {
  get: function get() {
    return this._writableState.autoDestroy;
  },
  set: function set(val) {
    this._writableState.autoDestroy = val;
  }
});
var openWriteFs = deprecate(function () {// Noop.
}, 'WriteStream.prototype.open() is deprecated', 'DEP0135');
WriteStream.prototype.open = openWriteFs;
WriteStream.prototype._construct = _construct;

WriteStream.prototype._write = function (data, encoding, cb) {
  var _this3 = this;

  this[kIsPerformingIO] = true;
  this[kFs].write(this.fd, data, 0, data.length, this.pos, function (er, bytes) {
    _this3[kIsPerformingIO] = false;

    if (_this3.destroyed) {
      // Tell ._destroy() that it's safe to close the fd now.
      cb(er);
      return _this3.emit(kIoDone, er);
    }

    if (er) {
      return cb(er);
    }

    _this3.bytesWritten += bytes;
    cb();
  });
  if (this.pos !== undefined) this.pos += data.length;
};

WriteStream.prototype._writev = function (data, cb) {
  var _this4 = this;

  var len = data.length;
  var chunks = new Array(len);
  var size = 0;

  for (var i = 0; i < len; i++) {
    var chunk = data[i].chunk;
    chunks[i] = chunk;
    size += chunk.length;
  }

  this[kIsPerformingIO] = true;
  this[kFs].writev(this.fd, chunks, this.pos, function (er, bytes) {
    _this4[kIsPerformingIO] = false;

    if (_this4.destroyed) {
      // Tell ._destroy() that it's safe to close the fd now.
      cb(er);
      return _this4.emit(kIoDone, er);
    }

    if (er) {
      return cb(er);
    }

    _this4.bytesWritten += bytes;
    cb();
  });
  if (this.pos !== undefined) this.pos += size;
};

WriteStream.prototype._destroy = function (err, cb) {
  var _this5 = this;

  // Usually for async IO it is safe to close a file descriptor
  // even when there are pending operations. However, due to platform
  // differences file IO is implemented using synchronous operations
  // running in a thread pool. Therefore, file descriptors are not safe
  // to close while used in a pending read or write operation. Wait for
  // any pending IO (kIsPerformingIO) to complete (kIoDone).
  if (this[kIsPerformingIO]) {
    this.once(kIoDone, function (er) {
      return close(_this5, err || er, cb);
    });
  } else {
    close(this, err, cb);
  }
};

WriteStream.prototype.close = function (cb) {
  if (cb) {
    if (this.closed) {
      process.nextTick(cb);
      return;
    }

    this.on('close', cb);
  } // If we are not autoClosing, we should call
  // destroy on 'finish'.


  if (!this.autoClose) {
    this.on('finish', this.destroy);
  } // We use end() instead of destroy() because of
  // https://github.com/nodejs/node/issues/2006


  this.end();
}; // There is no shutdown() for files.


WriteStream.prototype.destroySoon = WriteStream.prototype.end;
ObjectDefineProperty(WriteStream.prototype, 'pending', {
  get: function get() {
    return this.fd === null;
  },
  configurable: true
});
module.exports = {
  ReadStream: ReadStream,
  WriteStream: WriteStream
};
