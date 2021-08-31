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
// A bit simpler than readable streams.
// Implement an async ._write(chunk, encoding, cb), and it'll handle all
// the drain event emission and buffering.
'use strict';

var _primordials = primordials,
    ArrayPrototypeSlice = _primordials.ArrayPrototypeSlice,
    Error = _primordials.Error,
    FunctionPrototypeSymbolHasInstance = _primordials.FunctionPrototypeSymbolHasInstance,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    ObjectDefineProperties = _primordials.ObjectDefineProperties,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    StringPrototypeToLowerCase = _primordials.StringPrototypeToLowerCase,
    _Symbol = _primordials.Symbol,
    SymbolHasInstance = _primordials.SymbolHasInstance;
module.exports = Writable;
Writable.WritableState = WritableState;

var EE = require('events');

var Stream = require('internal/streams/legacy').Stream;

var _require = require('buffer'),
    Buffer = _require.Buffer;

var destroyImpl = require('internal/streams/destroy');

var _require2 = require('internal/streams/add-abort-signal'),
    addAbortSignal = _require2.addAbortSignal;

var _require3 = require('internal/streams/state'),
    getHighWaterMark = _require3.getHighWaterMark,
    getDefaultHighWaterMark = _require3.getDefaultHighWaterMark;

var _require$codes = require('internal/errors').codes,
    ERR_INVALID_ARG_TYPE = _require$codes.ERR_INVALID_ARG_TYPE,
    ERR_METHOD_NOT_IMPLEMENTED = _require$codes.ERR_METHOD_NOT_IMPLEMENTED,
    ERR_MULTIPLE_CALLBACK = _require$codes.ERR_MULTIPLE_CALLBACK,
    ERR_STREAM_CANNOT_PIPE = _require$codes.ERR_STREAM_CANNOT_PIPE,
    ERR_STREAM_DESTROYED = _require$codes.ERR_STREAM_DESTROYED,
    ERR_STREAM_ALREADY_FINISHED = _require$codes.ERR_STREAM_ALREADY_FINISHED,
    ERR_STREAM_NULL_VALUES = _require$codes.ERR_STREAM_NULL_VALUES,
    ERR_STREAM_WRITE_AFTER_END = _require$codes.ERR_STREAM_WRITE_AFTER_END,
    ERR_UNKNOWN_ENCODING = _require$codes.ERR_UNKNOWN_ENCODING;

var errorOrDestroy = destroyImpl.errorOrDestroy;
ObjectSetPrototypeOf(Writable.prototype, Stream.prototype);
ObjectSetPrototypeOf(Writable, Stream);

function nop() {}

var kOnFinished = _Symbol('kOnFinished');

function WritableState(options, stream, isDuplex) {
  // Duplex streams are both readable and writable, but share
  // the same options object.
  // However, some cases require setting options to different
  // values for the readable and the writable sides of the duplex stream,
  // e.g. options.readableObjectMode vs. options.writableObjectMode, etc.
  if (typeof isDuplex !== 'boolean') isDuplex = stream instanceof Stream.Duplex; // Object stream flag to indicate whether or not this stream
  // contains buffers or objects.

  this.objectMode = !!(options && options.objectMode);
  if (isDuplex) this.objectMode = this.objectMode || !!(options && options.writableObjectMode); // The point at which write() starts returning false
  // Note: 0 is a valid value, means that we always return false if
  // the entire buffer is not flushed immediately on write().

  this.highWaterMark = options ? getHighWaterMark(this, options, 'writableHighWaterMark', isDuplex) : getDefaultHighWaterMark(false); // if _final has been called.

  this.finalCalled = false; // drain event flag.

  this.needDrain = false; // At the start of calling end()

  this.ending = false; // When end() has been called, and returned.

  this.ended = false; // When 'finish' is emitted.

  this.finished = false; // Has it been destroyed

  this.destroyed = false; // Should we decode strings into buffers before passing to _write?
  // this is here so that some node-core streams can optimize string
  // handling at a lower level.

  var noDecode = !!(options && options.decodeStrings === false);
  this.decodeStrings = !noDecode; // Crypto is kind of old and crusty.  Historically, its default string
  // encoding is 'binary' so we have to make this configurable.
  // Everything else in the universe uses 'utf8', though.

  this.defaultEncoding = options && options.defaultEncoding || 'utf8'; // Not an actual buffer we keep track of, but a measurement
  // of how much we're waiting to get pushed to some underlying
  // socket or file.

  this.length = 0; // A flag to see when we're in the middle of a write.

  this.writing = false; // When true all writes will be buffered until .uncork() call.

  this.corked = 0; // A flag to be able to tell if the onwrite cb is called immediately,
  // or on a later tick.  We set this to true at first, because any
  // actions that shouldn't happen until "later" should generally also
  // not happen before the first write call.

  this.sync = true; // A flag to know if we're processing previously buffered items, which
  // may call the _write() callback in the same tick, so that we don't
  // end up in an overlapped onwrite situation.

  this.bufferProcessing = false; // The callback that's passed to _write(chunk, cb).

  this.onwrite = onwrite.bind(undefined, stream); // The callback that the user supplies to write(chunk, encoding, cb).

  this.writecb = null; // The amount that is being written when _write is called.

  this.writelen = 0; // Storage for data passed to the afterWrite() callback in case of
  // synchronous _write() completion.

  this.afterWriteTickInfo = null;
  resetBuffer(this); // Number of pending user-supplied write callbacks
  // this must be 0 before 'finish' can be emitted.

  this.pendingcb = 0; // Stream is still being constructed and cannot be
  // destroyed until construction finished or failed.
  // Async construction is opt in, therefore we start as
  // constructed.

  this.constructed = true; // Emit prefinish if the only thing we're waiting for is _write cbs
  // This is relevant for synchronous Transform streams.

  this.prefinished = false; // True if the error was already emitted and should not be thrown again.

  this.errorEmitted = false; // Should close be emitted on destroy. Defaults to true.

  this.emitClose = !options || options.emitClose !== false; // Should .destroy() be called after 'finish' (and potentially 'end').

  this.autoDestroy = !options || options.autoDestroy !== false; // Indicates whether the stream has errored. When true all write() calls
  // should return false. This is needed since when autoDestroy
  // is disabled we need a way to tell whether the stream has failed.

  this.errored = null; // Indicates whether the stream has finished destroying.

  this.closed = false; // True if close has been emitted or would have been emitted
  // depending on emitClose.

  this.closeEmitted = false;
  this[kOnFinished] = [];
}

function resetBuffer(state) {
  state.buffered = [];
  state.bufferedIndex = 0;
  state.allBuffers = true;
  state.allNoop = true;
}

WritableState.prototype.getBuffer = function getBuffer() {
  return ArrayPrototypeSlice(this.buffered, this.bufferedIndex);
};

ObjectDefineProperty(WritableState.prototype, 'bufferedRequestCount', {
  get: function get() {
    return this.buffered.length - this.bufferedIndex;
  }
});

function Writable(options) {
  var _this = this;

  // Writable ctor is applied to Duplexes, too.
  // `realHasInstance` is necessary because using plain `instanceof`
  // would return false, as no `_writableState` property is attached.
  // Trying to use the custom `instanceof` for Writable here will also break the
  // Node.js LazyTransform implementation, which has a non-trivial getter for
  // `_writableState` that would lead to infinite recursion.
  // Checking for a Stream.Duplex instance is faster here instead of inside
  // the WritableState constructor, at least with V8 6.5.
  var isDuplex = this instanceof Stream.Duplex;
  if (!isDuplex && !FunctionPrototypeSymbolHasInstance(Writable, this)) return new Writable(options);
  this._writableState = new WritableState(options, this, isDuplex);

  if (options) {
    if (typeof options.write === 'function') this._write = options.write;
    if (typeof options.writev === 'function') this._writev = options.writev;
    if (typeof options.destroy === 'function') this._destroy = options.destroy;
    if (typeof options["final"] === 'function') this._final = options["final"];
    if (typeof options.construct === 'function') this._construct = options.construct;
    if (options.signal) addAbortSignal(options.signal, this);
  }

  Stream.call(this, options);
  destroyImpl.construct(this, function () {
    var state = _this._writableState;

    if (!state.writing) {
      clearBuffer(_this, state);
    }

    finishMaybe(_this, state);
  });
}

ObjectDefineProperty(Writable, SymbolHasInstance, {
  value: function value(object) {
    if (FunctionPrototypeSymbolHasInstance(this, object)) return true;
    if (this !== Writable) return false;
    return object && object._writableState instanceof WritableState;
  }
}); // Otherwise people can pipe Writable streams, which is just wrong.

Writable.prototype.pipe = function () {
  errorOrDestroy(this, new ERR_STREAM_CANNOT_PIPE());
};

function _write(stream, chunk, encoding, cb) {
  var state = stream._writableState;

  if (typeof encoding === 'function') {
    cb = encoding;
    encoding = state.defaultEncoding;
  } else {
    if (!encoding) encoding = state.defaultEncoding;else if (encoding !== 'buffer' && !Buffer.isEncoding(encoding)) throw new ERR_UNKNOWN_ENCODING(encoding);
    if (typeof cb !== 'function') cb = nop;
  }

  if (chunk === null) {
    throw new ERR_STREAM_NULL_VALUES();
  } else if (!state.objectMode) {
    if (typeof chunk === 'string') {
      if (state.decodeStrings !== false) {
        chunk = Buffer.from(chunk, encoding);
        encoding = 'buffer';
      }
    } else if (chunk instanceof Buffer) {
      encoding = 'buffer';
    } else if (Stream._isUint8Array(chunk)) {
      chunk = Stream._uint8ArrayToBuffer(chunk);
      encoding = 'buffer';
    } else {
      throw new ERR_INVALID_ARG_TYPE('chunk', ['string', 'Buffer', 'Uint8Array'], chunk);
    }
  }

  var err;

  if (state.ending) {
    err = new ERR_STREAM_WRITE_AFTER_END();
  } else if (state.destroyed) {
    err = new ERR_STREAM_DESTROYED('write');
  }

  if (err) {
    process.nextTick(cb, err);
    errorOrDestroy(stream, err, true);
    return err;
  }

  state.pendingcb++;
  return writeOrBuffer(stream, state, chunk, encoding, cb);
}

Writable.prototype.write = function (chunk, encoding, cb) {
  return _write(this, chunk, encoding, cb) === true;
};

Writable.prototype.cork = function () {
  this._writableState.corked++;
};

Writable.prototype.uncork = function () {
  var state = this._writableState;

  if (state.corked) {
    state.corked--;
    if (!state.writing) clearBuffer(this, state);
  }
};

Writable.prototype.setDefaultEncoding = function setDefaultEncoding(encoding) {
  // node::ParseEncoding() requires lower case.
  if (typeof encoding === 'string') encoding = StringPrototypeToLowerCase(encoding);
  if (!Buffer.isEncoding(encoding)) throw new ERR_UNKNOWN_ENCODING(encoding);
  this._writableState.defaultEncoding = encoding;
  return this;
}; // If we're already writing something, then just put this
// in the queue, and wait our turn.  Otherwise, call _write
// If we return false, then we need a drain event, so set that flag.


function writeOrBuffer(stream, state, chunk, encoding, callback) {
  var len = state.objectMode ? 1 : chunk.length;
  state.length += len; // stream._write resets state.length

  var ret = state.length < state.highWaterMark; // We must ensure that previous needDrain will not be reset to false.

  if (!ret) state.needDrain = true;

  if (state.writing || state.corked || state.errored || !state.constructed) {
    state.buffered.push({
      chunk: chunk,
      encoding: encoding,
      callback: callback
    });

    if (state.allBuffers && encoding !== 'buffer') {
      state.allBuffers = false;
    }

    if (state.allNoop && callback !== nop) {
      state.allNoop = false;
    }
  } else {
    state.writelen = len;
    state.writecb = callback;
    state.writing = true;
    state.sync = true;

    stream._write(chunk, encoding, state.onwrite);

    state.sync = false;
  } // Return false if errored or destroyed in order to break
  // any synchronous while(stream.write(data)) loops.


  return ret && !state.errored && !state.destroyed;
}

function doWrite(stream, state, writev, len, chunk, encoding, cb) {
  state.writelen = len;
  state.writecb = cb;
  state.writing = true;
  state.sync = true;
  if (state.destroyed) state.onwrite(new ERR_STREAM_DESTROYED('write'));else if (writev) stream._writev(chunk, state.onwrite);else stream._write(chunk, encoding, state.onwrite);
  state.sync = false;
}

function onwriteError(stream, state, er, cb) {
  --state.pendingcb;
  cb(er); // Ensure callbacks are invoked even when autoDestroy is
  // not enabled. Passing `er` here doesn't make sense since
  // it's related to one specific write, not to the buffered
  // writes.

  errorBuffer(state); // This can emit error, but error must always follow cb.

  errorOrDestroy(stream, er);
}

function onwrite(stream, er) {
  var state = stream._writableState;
  var sync = state.sync;
  var cb = state.writecb;

  if (typeof cb !== 'function') {
    errorOrDestroy(stream, new ERR_MULTIPLE_CALLBACK());
    return;
  }
  state.writing = false;
  state.writecb = null;
  state.length -= state.writelen;
  state.writelen = 0;

  if (er) {
    // Avoid V8 leak, https://github.com/nodejs/node/pull/34103#issuecomment-652002364
    er.stack; // eslint-disable-line no-unused-expressions

    if (!state.errored) {
      state.errored = er;
    } // In case of duplex streams we need to notify the readable side of the
    // error.


    if (stream._readableState && !stream._readableState.errored) {
      stream._readableState.errored = er;
    }

    if (sync) {
      process.nextTick(onwriteError, stream, state, er, cb);
    } else {
      onwriteError(stream, state, er, cb);
    }
  } else {
    if (state.buffered.length > state.bufferedIndex) {
      clearBuffer(stream, state);
    }

    if (sync) {
      // It is a common case that the callback passed to .write() is always
      // the same. In that case, we do not schedule a new nextTick(), but
      // rather just increase a counter, to improve performance and avoid
      // memory allocations.
      if (state.afterWriteTickInfo !== null && state.afterWriteTickInfo.cb === cb) {
        state.afterWriteTickInfo.count++;
      } else {
        state.afterWriteTickInfo = {
          count: 1,
          cb: cb,
          stream: stream,
          state: state
        };
        // process.nextTick(afterWriteTick, state.afterWriteTickInfo);
        // Need to figure out the usefulness of this
      }
    } else {
      afterWrite(stream, state, 1, cb);
    }
  }
}

function afterWriteTick(_ref) {
  var stream = _ref.stream,
      state = _ref.state,
      count = _ref.count,
      cb = _ref.cb;
  state.afterWriteTickInfo = null;
  return afterWrite(stream, state, count, cb);
}

function afterWrite(stream, state, count, cb) {
  var needDrain = !state.ending && !stream.destroyed && state.length === 0 && state.needDrain;

  if (needDrain) {
    state.needDrain = false;
    stream.emit('drain');
  }

  while (count-- > 0) {
    state.pendingcb--;
    cb();
  }

  if (state.destroyed) {
    errorBuffer(state);
  }

  finishMaybe(stream, state);
} // If there's something in the buffer waiting, then invoke callbacks.


function errorBuffer(state) {
  if (state.writing) {
    return;
  }

  for (var n = state.bufferedIndex; n < state.buffered.length; ++n) {
    var _state$buffered$n = state.buffered[n],
        chunk = _state$buffered$n.chunk,
        callback = _state$buffered$n.callback;
    var len = state.objectMode ? 1 : chunk.length;
    state.length -= len;
    callback(new ERR_STREAM_DESTROYED('write'));
  }

  var onfinishCallbacks = state[kOnFinished].splice(0);

  for (var i = 0; i < onfinishCallbacks.length; i++) {
    onfinishCallbacks[i](new ERR_STREAM_DESTROYED('end'));
  }

  resetBuffer(state);
} // If there's something in the buffer waiting, then process it.


function clearBuffer(stream, state) {
  if (state.corked || state.bufferProcessing || state.destroyed || !state.constructed) {
    return;
  }

  var buffered = state.buffered,
      bufferedIndex = state.bufferedIndex,
      objectMode = state.objectMode;
  var bufferedLength = buffered.length - bufferedIndex;

  if (!bufferedLength) {
    return;
  }

  var i = bufferedIndex;
  state.bufferProcessing = true;

  if (bufferedLength > 1 && stream._writev) {
    state.pendingcb -= bufferedLength - 1;
    var callback = state.allNoop ? nop : function (err) {
      for (var n = i; n < buffered.length; ++n) {
        buffered[n].callback(err);
      }
    }; // Make a copy of `buffered` if it's going to be used by `callback` above,
    // since `doWrite` will mutate the array.

    var chunks = state.allNoop && i === 0 ? buffered : ArrayPrototypeSlice(buffered, i);
    chunks.allBuffers = state.allBuffers;
    doWrite(stream, state, true, state.length, chunks, '', callback);
    resetBuffer(state);
  } else {
    do {
      var _buffered$i = buffered[i],
          chunk = _buffered$i.chunk,
          encoding = _buffered$i.encoding,
          _callback = _buffered$i.callback;
      buffered[i++] = null;
      var len = objectMode ? 1 : chunk.length;
      doWrite(stream, state, false, len, chunk, encoding, _callback);
    } while (i < buffered.length && !state.writing);

    if (i === buffered.length) {
      resetBuffer(state);
    } else if (i > 256) {
      buffered.splice(0, i);
      state.bufferedIndex = 0;
    } else {
      state.bufferedIndex = i;
    }
  }

  state.bufferProcessing = false;
}

Writable.prototype._write = function (chunk, encoding, cb) {
  if (this._writev) {
    this._writev([{
      chunk: chunk,
      encoding: encoding
    }], cb);
  } else {
    throw new ERR_METHOD_NOT_IMPLEMENTED('_write()');
  }
};

Writable.prototype._writev = null;

Writable.prototype.end = function (chunk, encoding, cb) {
  var state = this._writableState;

  if (typeof chunk === 'function') {
    cb = chunk;
    chunk = null;
    encoding = null;
  } else if (typeof encoding === 'function') {
    cb = encoding;
    encoding = null;
  }

  var err;

  if (chunk !== null && chunk !== undefined) {
    var ret = _write(this, chunk, encoding);

    if (ret instanceof Error) {
      err = ret;
    }
  } // .end() fully uncorks.


  if (state.corked) {
    state.corked = 1;
    this.uncork();
  }

  if (err) {// Do nothing...
  } else if (!state.errored && !state.ending) {
    // This is forgiving in terms of unnecessary calls to end() and can hide
    // logic errors. However, usually such errors are harmless and causing a
    // hard error can be disproportionately destructive. It is not always
    // trivial for the user to determine whether end() needs to be called
    // or not.
    state.ending = true;
    finishMaybe(this, state, true);
    state.ended = true;
  } else if (state.finished) {
    err = new ERR_STREAM_ALREADY_FINISHED('end');
  } else if (state.destroyed) {
    err = new ERR_STREAM_DESTROYED('end');
  }

  if (typeof cb === 'function') {
    if (err || state.finished) {
      process.nextTick(cb, err);
    } else {
      state[kOnFinished].push(cb);
    }
  }

  return this;
};

function needFinish(state) {
  return state.ending && state.constructed && state.length === 0 && !state.errored && state.buffered.length === 0 && !state.finished && !state.writing && !state.errorEmitted && !state.closeEmitted;
}

function callFinal(stream, state) {
  var called = false;

  function onFinish(err) {
    if (called) {
      errorOrDestroy(stream, err !== null && err !== void 0 ? err : ERR_MULTIPLE_CALLBACK());
      return;
    }

    called = true;
    state.pendingcb--;

    if (err) {
      var onfinishCallbacks = state[kOnFinished].splice(0);

      for (var i = 0; i < onfinishCallbacks.length; i++) {
        onfinishCallbacks[i](err);
      }

      errorOrDestroy(stream, err, state.sync);
    } else if (needFinish(state)) {
      state.prefinished = true;
      stream.emit('prefinish'); // Backwards compat. Don't check state.sync here.
      // Some streams assume 'finish' will be emitted
      // asynchronously relative to _final callback.

      state.pendingcb++;
      process.nextTick(finish, stream, state);
    }
  }

  state.sync = true;
  state.pendingcb++;

  try {
    var result = stream._final(onFinish);

    if (result != null) {
      var then = result.then;

      if (typeof then === 'function') {
        then.call(result, function () {
          process.nextTick(onFinish, null);
        }, function (err) {
          process.nextTick(onFinish, err);
        });
      }
    }
  } catch (err) {
    onFinish(stream, state, err);
  }

  state.sync = false;
}

function prefinish(stream, state) {
  if (!state.prefinished && !state.finalCalled) {
    if (typeof stream._final === 'function' && !state.destroyed) {
      state.finalCalled = true;
      callFinal(stream, state);
    } else {
      state.prefinished = true;
      stream.emit('prefinish');
    }
  }
}

function finishMaybe(stream, state, sync) {
  if (needFinish(state)) {
    prefinish(stream, state);

    if (state.pendingcb === 0 && needFinish(state)) {
      state.pendingcb++;

      if (sync) {
        process.nextTick(finish, stream, state);
      } else {
        finish(stream, state);
      }
    }
  }
}

function finish(stream, state) {
  state.pendingcb--;
  state.finished = true;
  var onfinishCallbacks = state[kOnFinished].splice(0);

  for (var i = 0; i < onfinishCallbacks.length; i++) {
    onfinishCallbacks[i]();
  }

  stream.emit('finish');

  if (state.autoDestroy) {
    // In case of duplex streams we need a way to detect
    // if the readable side is ready for autoDestroy as well.
    var rState = stream._readableState;
    var autoDestroy = !rState || rState.autoDestroy && ( // We don't expect the readable to ever 'end'
    // if readable is explicitly set to false.
    rState.endEmitted || rState.readable === false);

    if (autoDestroy) {
      stream.destroy();
    }
  }
}

ObjectDefineProperties(Writable.prototype, {
  destroyed: {
    get: function get() {
      return this._writableState ? this._writableState.destroyed : false;
    },
    set: function set(value) {
      // Backward compatibility, the user is explicitly managing destroyed.
      if (this._writableState) {
        this._writableState.destroyed = value;
      }
    }
  },
  writable: {
    get: function get() {
      var w = this._writableState; // w.writable === false means that this is part of a Duplex stream
      // where the writable side was disabled upon construction.
      // Compat. The user might manually disable writable side through
      // deprecated setter.

      return !!w && w.writable !== false && !w.destroyed && !w.errored && !w.ending && !w.ended;
    },
    set: function set(val) {
      // Backwards compatible.
      if (this._writableState) {
        this._writableState.writable = !!val;
      }
    }
  },
  writableFinished: {
    get: function get() {
      return this._writableState ? this._writableState.finished : false;
    }
  },
  writableObjectMode: {
    get: function get() {
      return this._writableState ? this._writableState.objectMode : false;
    }
  },
  writableBuffer: {
    get: function get() {
      return this._writableState && this._writableState.getBuffer();
    }
  },
  writableEnded: {
    get: function get() {
      return this._writableState ? this._writableState.ending : false;
    }
  },
  writableNeedDrain: {
    get: function get() {
      var wState = this._writableState;
      if (!wState) return false;
      return !wState.destroyed && !wState.ending && wState.needDrain;
    }
  },
  writableHighWaterMark: {
    get: function get() {
      return this._writableState && this._writableState.highWaterMark;
    }
  },
  writableCorked: {
    get: function get() {
      return this._writableState ? this._writableState.corked : 0;
    }
  },
  writableLength: {
    get: function get() {
      return this._writableState && this._writableState.length;
    }
  }
});
var destroy = destroyImpl.destroy;

Writable.prototype.destroy = function (err, cb) {
  var state = this._writableState; // Invoke pending callbacks.

  if (!state.destroyed && (state.bufferedIndex < state.buffered.length || state[kOnFinished].length)) {
    process.nextTick(errorBuffer, state);
  }

  destroy.call(this, err, cb);
  return this;
};

Writable.prototype._undestroy = destroyImpl.undestroy;

Writable.prototype._destroy = function (err, cb) {
  cb(err);
};

Writable.prototype[EE.captureRejectionSymbol] = function (err) {
  this.destroy(err);
};
