'use strict';

var _primordials = primordials,
    Array = _primordials.Array,
    _Symbol = _primordials.Symbol;
var _require = require('buffer'),
    Buffer = _require.Buffer;

var _require2 = require('internal/buffer'),
    FastBuffer = _require2.FastBuffer;

// var _internalBinding = internalBinding('stream_wrap'),
//     WriteWrap = _internalBinding.WriteWrap;
//     kReadBytesOrError = _internalBinding.kReadBytesOrError,
//     kArrayBufferOffset = _internalBinding.kArrayBufferOffset,
//     kBytesWritten = _internalBinding.kBytesWritten,
//     kLastWriteWasAsync = _internalBinding.kLastWriteWasAsync,
    // streamBaseState = _internalBinding.streamBaseState;

// var _internalBinding2 = internalBinding('uv'),
//     UV_EOF = _internalBinding2.UV_EOF;

var _require3 = require('internal/errors'),
    errnoException = _require3.errnoException;

// var owner_symbol = require('internal/async_hooks').symbols.owner_symbol;

// var _require4 = require('internal/timers'),
//     kTimeout = _require4.kTimeout,
//     setUnrefTimeout = _require4.setUnrefTimeout,
//     getTimerDuration = _require4.getTimerDuration;

var _require5 = require('internal/util/types'),
    isUint8Array = _require5.isUint8Array;

// var _require6 = require('timers'),
//     clearTimeout = _require6.clearTimeout;

var _require7 = require('internal/validators'),
    validateCallback = _require7.validateCallback;

var kMaybeDestroy = _Symbol('kMaybeDestroy');

var kUpdateTimer = _Symbol('kUpdateTimer');

var kAfterAsyncWrite = _Symbol('kAfterAsyncWrite');

var kHandle = _Symbol('kHandle');

var kSession = _Symbol('kSession');

var debug = require('internal/util/debuglog').debuglog('stream', function (fn) {
  debug = fn;
});

var kBuffer = _Symbol('kBuffer');

var kBufferGen = _Symbol('kBufferGen');

var kBufferCb = _Symbol('kBufferCb');

function handleWriteReq(req, data, encoding) {
  var handle = req.handle;

  switch (encoding) {
    case 'buffer':
      {
        var ret = handle.writeBuffer(req, data);
        if (streamBaseState[kLastWriteWasAsync]) req.buffer = data;
        return ret;
      }

    case 'latin1':
    case 'binary':
      return handle.writeLatin1String(req, data);

    case 'utf8':
    case 'utf-8':
      return handle.writeUtf8String(req, data);

    case 'ascii':
      return handle.writeAsciiString(req, data);

    case 'ucs2':
    case 'ucs-2':
    case 'utf16le':
    case 'utf-16le':
      return handle.writeUcs2String(req, data);

    default:
      {
        var buffer = Buffer.from(data, encoding);

        var _ret = handle.writeBuffer(req, buffer);

        if (streamBaseState[kLastWriteWasAsync]) req.buffer = buffer;
        return _ret;
      }
  }
}

function onWriteComplete(status) {
  debug('onWriteComplete', status, this.error);
  var stream = this.handle[owner_symbol];

  if (stream.destroyed) {
    if (typeof this.callback === 'function') this.callback(null);
    return;
  } // TODO (ronag): This should be moved before if(stream.destroyed)
  // in order to avoid swallowing error.


  if (status < 0) {
    var ex = errnoException(status, 'write', this.error);
    if (typeof this.callback === 'function') this.callback(ex);else stream.destroy(ex);
    return;
  }

  stream[kUpdateTimer]();
  stream[kAfterAsyncWrite](this);
  if (typeof this.callback === 'function') this.callback(null);
}

function createWriteWrap(handle, callback) {
  var req = {}; //new WriteWrap();
  req.handle = handle;
  req.oncomplete = onWriteComplete;
  req.async = false;
  req.bytes = 0;
  req.buffer = null;
  req.callback = callback;
  return req;
}

function writevGeneric(self, data, cb) {
  var req = createWriteWrap(self[kHandle], cb);
  var allBuffers = data.allBuffers;
  var chunks;

  if (allBuffers) {
    chunks = data;

    for (var i = 0; i < data.length; i++) {
      data[i] = data[i].chunk;
    }
  } else {
    chunks = new Array(data.length << 1);

    for (var _i = 0; _i < data.length; _i++) {
      var entry = data[_i];
      chunks[_i * 2] = entry.chunk;
      chunks[_i * 2 + 1] = entry.encoding;
    }
  }

  var err = req.handle.writev(req, chunks, allBuffers); // Retain chunks

  if (err === 0) req._chunks = chunks;
  afterWriteDispatched(req, err, cb);
  return req;
}

function writeGeneric(self, data, encoding, cb) {
  var req = createWriteWrap(self[kHandle], cb);
  var err = handleWriteReq(req, data, encoding);
  afterWriteDispatched(req, err, cb);
  return req;
}

function afterWriteDispatched(req, err, cb) {
  // req.bytes = streamBaseState[kBytesWritten];
  req.async = false; //!!streamBaseState[kLastWriteWasAsync];
  if (err !== 0) return cb(errnoException(err, 'write', req.error));

  if (!req.async && typeof req.callback === 'function') {
    req.callback();
  }
}

function onStreamRead(arrayBuffer) {
  var nread = streamBaseState[kReadBytesOrError];
  var handle = this;
  var stream = this[owner_symbol];
  stream[kUpdateTimer]();

  if (nread > 0 && !stream.destroyed) {
    var ret;
    var result;
    var userBuf = stream[kBuffer];

    if (userBuf) {
      result = stream[kBufferCb](nread, userBuf) !== false;
      var bufGen = stream[kBufferGen];

      if (bufGen !== null) {
        var nextBuf = bufGen();
        if (isUint8Array(nextBuf)) stream[kBuffer] = ret = nextBuf;
      }
    } else {
      var offset = streamBaseState[kArrayBufferOffset];
      var buf = new FastBuffer(arrayBuffer, offset, nread);
      result = stream.push(buf);
    }

    if (!result) {
      handle.reading = false;

      if (!stream.destroyed) {
        var err = handle.readStop();
        if (err) stream.destroy(errnoException(err, 'read'));
      }
    }

    return ret;
  }

  if (nread === 0) {
    return;
  }

  if (nread !== UV_EOF) {
    // CallJSOnreadMethod expects the return value to be a buffer.
    // Ref: https://github.com/nodejs/node/pull/34375
    stream.destroy(errnoException(nread, 'read'));
    return;
  } // Defer this until we actually emit end


  if (stream._readableState.endEmitted) {
    if (stream[kMaybeDestroy]) stream[kMaybeDestroy]();
  } else {
    if (stream[kMaybeDestroy]) stream.on('end', stream[kMaybeDestroy]); // TODO(ronag): Without this `readStop`, `onStreamRead`
    // will be called once more (i.e. after Readable.ended)
    // on Windows causing a ECONNRESET, failing the
    // test-https-truncate test.

    if (handle.readStop) {
      var _err = handle.readStop();

      if (_err) {
        // CallJSOnreadMethod expects the return value to be a buffer.
        // Ref: https://github.com/nodejs/node/pull/34375
        stream.destroy(errnoException(_err, 'read'));
        return;
      }
    } // Push a null to signal the end of data.
    // Do it before `maybeDestroy` for correct order of events:
    // `end` -> `close`


    stream.push(null);
    stream.read(0);
  }
}

function setStreamTimeout(msecs, callback) {
  if (this.destroyed) return this;
  this.timeout = msecs; // Type checking identical to timers.enroll()

  msecs = getTimerDuration(msecs, 'msecs'); // Attempt to clear an existing timer in both cases -
  //  even if it will be rescheduled we don't want to leak an existing timer.

  clearTimeout(this[kTimeout]);

  if (msecs === 0) {
    if (callback !== undefined) {
      validateCallback(callback);
      this.removeListener('timeout', callback);
    }
  } else {
    this[kTimeout] = setUnrefTimeout(this._onTimeout.bind(this), msecs);
    if (this[kSession]) this[kSession][kUpdateTimer]();

    if (callback !== undefined) {
      validateCallback(callback);
      this.once('timeout', callback);
    }
  }

  return this;
}

module.exports = {
  writevGeneric: writevGeneric,
  writeGeneric: writeGeneric,
  onStreamRead: onStreamRead,
  kAfterAsyncWrite: kAfterAsyncWrite,
  kMaybeDestroy: kMaybeDestroy,
  kUpdateTimer: kUpdateTimer,
  kHandle: kHandle,
  kSession: kSession,
  setStreamTimeout: setStreamTimeout,
  kBuffer: kBuffer,
  kBufferCb: kBufferCb,
  kBufferGen: kBufferGen
};
