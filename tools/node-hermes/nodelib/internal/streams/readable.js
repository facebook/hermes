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
'use strict';

function _createForOfIteratorHelper(o, allowArrayLike) { var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"]; if (!it) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = it.call(o); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function _awaitAsyncGenerator(value) { return new _AwaitValue(value); }

function _wrapAsyncGenerator(fn) { return function () { return new _AsyncGenerator(fn.apply(this, arguments)); }; }

function _AsyncGenerator(gen) { var front, back; function send(key, arg) { return new Promise(function (resolve, reject) { var request = { key: key, arg: arg, resolve: resolve, reject: reject, next: null }; if (back) { back = back.next = request; } else { front = back = request; resume(key, arg); } }); } function resume(key, arg) { try { var result = gen[key](arg); var value = result.value; var wrappedAwait = value instanceof _AwaitValue; Promise.resolve(wrappedAwait ? value.wrapped : value).then(function (arg) { if (wrappedAwait) { resume(key === "return" ? "return" : "next", arg); return; } settle(result.done ? "return" : "normal", arg); }, function (err) { resume("throw", err); }); } catch (err) { settle("throw", err); } } function settle(type, value) { switch (type) { case "return": front.resolve({ value: value, done: true }); break; case "throw": front.reject(value); break; default: front.resolve({ value: value, done: false }); break; } front = front.next; if (front) { resume(front.key, front.arg); } else { back = null; } } this._invoke = send; if (typeof gen["return"] !== "function") { this["return"] = undefined; } }

_AsyncGenerator.prototype[typeof Symbol === "function" && Symbol.asyncIterator || "@@asyncIterator"] = function () { return this; };

_AsyncGenerator.prototype.next = function (arg) { return this._invoke("next", arg); };

_AsyncGenerator.prototype["throw"] = function (arg) { return this._invoke("throw", arg); };

_AsyncGenerator.prototype["return"] = function (arg) { return this._invoke("return", arg); };

function _AwaitValue(value) { this.wrapped = value; }

var _primordials = primordials,
    ArrayPrototypeIndexOf = _primordials.ArrayPrototypeIndexOf,
    NumberIsInteger = _primordials.NumberIsInteger,
    NumberIsNaN = _primordials.NumberIsNaN,
    NumberParseInt = _primordials.NumberParseInt,
    ObjectDefineProperties = _primordials.ObjectDefineProperties,
    ObjectKeys = _primordials.ObjectKeys,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    _Promise = _primordials.Promise,
    SafeSet = _primordials.SafeSet,
    SymbolAsyncIterator = _primordials.SymbolAsyncIterator,
    _Symbol = _primordials.Symbol;
module.exports = Readable;
Readable.ReadableState = ReadableState;

var EE = require('events');

var _require = require('internal/streams/legacy'),
    Stream = _require.Stream,
    prependListener = _require.prependListener;

var _require2 = require('buffer'),
    Buffer = _require2.Buffer;

var _require3 = require('internal/streams/add-abort-signal'),
    addAbortSignal = _require3.addAbortSignal;

var eos = require('internal/streams/end-of-stream');

var debug = require('internal/util/debuglog').debuglog('stream', function (fn) {
  debug = fn;
});

var BufferList = require('internal/streams/buffer_list');

var destroyImpl = require('internal/streams/destroy');

var _require4 = require('internal/streams/state'),
    getHighWaterMark = _require4.getHighWaterMark,
    getDefaultHighWaterMark = _require4.getDefaultHighWaterMark;

var _require5 = require('internal/errors'),
    aggregateTwoErrors = _require5.aggregateTwoErrors,
    _require5$codes = _require5.codes,
    ERR_INVALID_ARG_TYPE = _require5$codes.ERR_INVALID_ARG_TYPE,
    ERR_METHOD_NOT_IMPLEMENTED = _require5$codes.ERR_METHOD_NOT_IMPLEMENTED,
    ERR_STREAM_PUSH_AFTER_EOF = _require5$codes.ERR_STREAM_PUSH_AFTER_EOF,
    ERR_STREAM_UNSHIFT_AFTER_END_EVENT = _require5$codes.ERR_STREAM_UNSHIFT_AFTER_END_EVENT;

var _require6 = require('internal/validators'),
    validateObject = _require6.validateObject;

var kPaused = _Symbol('kPaused');

// var _require7 = require('string_decoder'),
//     StringDecoder = _require7.StringDecoder;

var from = require('internal/streams/from');

ObjectSetPrototypeOf(Readable.prototype, Stream.prototype);
ObjectSetPrototypeOf(Readable, Stream);

var nop = function nop() {};

var errorOrDestroy = destroyImpl.errorOrDestroy;

function ReadableState(options, stream, isDuplex) {
  // Duplex streams are both readable and writable, but share
  // the same options object.
  // However, some cases require setting options to different
  // values for the readable and the writable sides of the duplex stream.
  // These options can be provided separately as readableXXX and writableXXX.
  if (typeof isDuplex !== 'boolean') isDuplex = stream instanceof Stream.Duplex; // Object stream flag. Used to make read(n) ignore n and to
  // make all the buffer merging and length checks go away.

  this.objectMode = !!(options && options.objectMode);
  if (isDuplex) this.objectMode = this.objectMode || !!(options && options.readableObjectMode); // The point at which it stops calling _read() to fill the buffer
  // Note: 0 is a valid value, means "don't call _read preemptively ever"

  this.highWaterMark = options ? getHighWaterMark(this, options, 'readableHighWaterMark', isDuplex) : getDefaultHighWaterMark(false); // A linked list is used to store data chunks instead of an array because the
  // linked list can remove elements from the beginning faster than
  // array.shift().

  this.buffer = new BufferList();
  this.length = 0;
  this.pipes = [];
  this.flowing = null;
  this.ended = false;
  this.endEmitted = false;
  this.reading = false; // Stream is still being constructed and cannot be
  // destroyed until construction finished or failed.
  // Async construction is opt in, therefore we start as
  // constructed.

  this.constructed = true; // A flag to be able to tell if the event 'readable'/'data' is emitted
  // immediately, or on a later tick.  We set this to true at first, because
  // any actions that shouldn't happen until "later" should generally also
  // not happen before the first read call.

  this.sync = true; // Whenever we return null, then we set a flag to say
  // that we're awaiting a 'readable' event emission.

  this.needReadable = false;
  this.emittedReadable = false;
  this.readableListening = false;
  this.resumeScheduled = false;
  this[kPaused] = null; // True if the error was already emitted and should not be thrown again.

  this.errorEmitted = false; // Should close be emitted on destroy. Defaults to true.

  this.emitClose = !options || options.emitClose !== false; // Should .destroy() be called after 'end' (and potentially 'finish').

  this.autoDestroy = !options || options.autoDestroy !== false; // Has it been destroyed.

  this.destroyed = false; // Indicates whether the stream has errored. When true no further
  // _read calls, 'data' or 'readable' events should occur. This is needed
  // since when autoDestroy is disabled we need a way to tell whether the
  // stream has failed.

  this.errored = null; // Indicates whether the stream has finished destroying.

  this.closed = false; // True if close has been emitted or would have been emitted
  // depending on emitClose.

  this.closeEmitted = false; // Crypto is kind of old and crusty.  Historically, its default string
  // encoding is 'binary' so we have to make this configurable.
  // Everything else in the universe uses 'utf8', though.

  this.defaultEncoding = options && options.defaultEncoding || 'utf8'; // Ref the piped dest which we need a drain event on it
  // type: null | Writable | Set<Writable>.

  this.awaitDrainWriters = null;
  this.multiAwaitDrain = false; // If true, a maybeReadMore has been scheduled.

  this.readingMore = false;
  this.decoder = null;
  this.encoding = null;

  if (options && options.encoding) {
    this.decoder = new StringDecoder(options.encoding);
    this.encoding = options.encoding;
  }
}

function Readable(options) {
  var _this = this;

  if (!(this instanceof Readable)) return new Readable(options); // Checking for a Stream.Duplex instance is faster here instead of inside
  // the ReadableState constructor, at least with V8 6.5.

  var isDuplex = this instanceof Stream.Duplex;
  this._readableState = new ReadableState(options, this, isDuplex);

  if (options) {
    if (typeof options.read === 'function') this._read = options.read;
    if (typeof options.destroy === 'function') this._destroy = options.destroy;
    if (typeof options.construct === 'function') this._construct = options.construct;
    if (options.signal && !isDuplex) addAbortSignal(options.signal, this);
  }

  Stream.call(this, options);
  destroyImpl.construct(this, function () {
    if (_this._readableState.needReadable) {
      maybeReadMore(_this, _this._readableState);
    }
  });
}

Readable.prototype.destroy = destroyImpl.destroy;
Readable.prototype._undestroy = destroyImpl.undestroy;

Readable.prototype._destroy = function (err, cb) {
  cb(err);
};

Readable.prototype[EE.captureRejectionSymbol] = function (err) {
  this.destroy(err);
}; // Manually shove something into the read() buffer.
// This returns true if the highWaterMark has not been hit yet,
// similar to how Writable.write() returns true if you should
// write() some more.


Readable.prototype.push = function (chunk, encoding) {
  return readableAddChunk(this, chunk, encoding, false);
}; // Unshift should *always* be something directly out of read().


Readable.prototype.unshift = function (chunk, encoding) {
  return readableAddChunk(this, chunk, encoding, true);
};

function readableAddChunk(stream, chunk, encoding, addToFront) {
  debug('readableAddChunk', chunk);
  var state = stream._readableState;
  var err;

  if (!state.objectMode) {
    if (typeof chunk === 'string') {
      encoding = encoding || state.defaultEncoding;

      if (state.encoding !== encoding) {
        if (addToFront && state.encoding) {
          // When unshifting, if state.encoding is set, we have to save
          // the string in the BufferList with the state encoding.
          chunk = Buffer.from(chunk, encoding).toString(state.encoding);
        } else {
          chunk = Buffer.from(chunk, encoding);
          encoding = '';
        }
      }
    } else if (chunk instanceof Buffer) {
      encoding = '';
    } else if (Stream._isUint8Array(chunk)) {
      chunk = Stream._uint8ArrayToBuffer(chunk);
      encoding = '';
    } else if (chunk != null) {
      err = new ERR_INVALID_ARG_TYPE('chunk', ['string', 'Buffer', 'Uint8Array'], chunk);
    }
  }

  if (err) {
    errorOrDestroy(stream, err);
  } else if (chunk === null) {
    state.reading = false;
    onEofChunk(stream, state);
  } else if (state.objectMode || chunk && chunk.length > 0) {
    if (addToFront) {
      if (state.endEmitted) errorOrDestroy(stream, new ERR_STREAM_UNSHIFT_AFTER_END_EVENT());else addChunk(stream, state, chunk, true);
    } else if (state.ended) {
      errorOrDestroy(stream, new ERR_STREAM_PUSH_AFTER_EOF());
    } else if (state.destroyed || state.errored) {
      return false;
    } else {
      state.reading = false;

      if (state.decoder && !encoding) {
        chunk = state.decoder.write(chunk);
        if (state.objectMode || chunk.length !== 0) addChunk(stream, state, chunk, false);else maybeReadMore(stream, state);
      } else {
        addChunk(stream, state, chunk, false);
      }
    }
  } else if (!addToFront) {
    state.reading = false;
    maybeReadMore(stream, state);
  } // We can push more data if we are below the highWaterMark.
  // Also, if we have no data yet, we can stand some more bytes.
  // This is to work around cases where hwm=0, such as the repl.


  return !state.ended && (state.length < state.highWaterMark || state.length === 0);
}

function addChunk(stream, state, chunk, addToFront) {
  if (state.flowing && state.length === 0 && !state.sync && stream.listenerCount('data') > 0) {
    // Use the guard to avoid creating `Set()` repeatedly
    // when we have multiple pipes.
    if (state.multiAwaitDrain) {
      state.awaitDrainWriters.clear();
    } else {
      state.awaitDrainWriters = null;
    }

    stream.emit('data', chunk);
  } else {
    // Update the buffer info.
    state.length += state.objectMode ? 1 : chunk.length;
    if (addToFront) state.buffer.unshift(chunk);else state.buffer.push(chunk);
    if (state.needReadable) emitReadable(stream);
  }

  maybeReadMore(stream, state);
}

Readable.prototype.isPaused = function () {
  var state = this._readableState;
  return state[kPaused] === true || state.flowing === false;
}; // Backwards compatibility.


Readable.prototype.setEncoding = function (enc) {
  var decoder = new StringDecoder(enc);
  this._readableState.decoder = decoder; // If setEncoding(null), decoder.encoding equals utf8.

  this._readableState.encoding = this._readableState.decoder.encoding;
  var buffer = this._readableState.buffer; // Iterate over current buffer to convert already stored Buffers:

  var content = '';

  var _iterator = _createForOfIteratorHelper(buffer),
      _step;

  try {
    for (_iterator.s(); !(_step = _iterator.n()).done;) {
      var data = _step.value;
      content += decoder.write(data);
    }
  } catch (err) {
    _iterator.e(err);
  } finally {
    _iterator.f();
  }

  buffer.clear();
  if (content !== '') buffer.push(content);
  this._readableState.length = content.length;
  return this;
}; // Don't raise the hwm > 1GB.


var MAX_HWM = 0x40000000;

function computeNewHighWaterMark(n) {
  if (n >= MAX_HWM) {
    // TODO(ronag): Throw ERR_VALUE_OUT_OF_RANGE.
    n = MAX_HWM;
  } else {
    // Get the next highest power of 2 to prevent increasing hwm excessively in
    // tiny amounts.
    n--;
    n |= n >>> 1;
    n |= n >>> 2;
    n |= n >>> 4;
    n |= n >>> 8;
    n |= n >>> 16;
    n++;
  }

  return n;
} // This function is designed to be inlinable, so please take care when making
// changes to the function body.


function howMuchToRead(n, state) {
  if (n <= 0 || state.length === 0 && state.ended) return 0;
  if (state.objectMode) return 1;

  if (NumberIsNaN(n)) {
    // Only flow one buffer at a time.
    if (state.flowing && state.length) return state.buffer.first().length;
    return state.length;
  }

  if (n <= state.length) return n;
  return state.ended ? state.length : 0;
} // You can override either this method, or the async _read(n) below.


Readable.prototype.read = function (n) {
  debug('read', n); // Same as parseInt(undefined, 10), however V8 7.3 performance regressed
  // in this scenario, so we are doing it manually.

  if (n === undefined) {
    n = NaN;
  } else if (!NumberIsInteger(n)) {
    n = NumberParseInt(n, 10);
  }

  var state = this._readableState;
  var nOrig = n; // If we're asking for more than the current hwm, then raise the hwm.

  if (n > state.highWaterMark) state.highWaterMark = computeNewHighWaterMark(n);
  if (n !== 0) state.emittedReadable = false; // If we're doing read(0) to trigger a readable event, but we
  // already have a bunch of data in the buffer, then just trigger
  // the 'readable' event and move on.

  if (n === 0 && state.needReadable && ((state.highWaterMark !== 0 ? state.length >= state.highWaterMark : state.length > 0) || state.ended)) {
    debug('read: emitReadable', state.length, state.ended);
    if (state.length === 0 && state.ended) endReadable(this);else emitReadable(this);
    return null;
  }

  n = howMuchToRead(n, state); // If we've ended, and we're now clear, then finish it up.

  if (n === 0 && state.ended) {
    if (state.length === 0) endReadable(this);
    return null;
  } // All the actual chunk generation logic needs to be
  // *below* the call to _read.  The reason is that in certain
  // synthetic stream cases, such as passthrough streams, _read
  // may be a completely synchronous operation which may change
  // the state of the read buffer, providing enough data when
  // before there was *not* enough.
  //
  // So, the steps are:
  // 1. Figure out what the state of things will be after we do
  // a read from the buffer.
  //
  // 2. If that resulting state will trigger a _read, then call _read.
  // Note that this may be asynchronous, or synchronous.  Yes, it is
  // deeply ugly to write APIs this way, but that still doesn't mean
  // that the Readable class should behave improperly, as streams are
  // designed to be sync/async agnostic.
  // Take note if the _read call is sync or async (ie, if the read call
  // has returned yet), so that we know whether or not it's safe to emit
  // 'readable' etc.
  //
  // 3. Actually pull the requested chunks out of the buffer and return.
  // if we need a readable event, then we need to do some reading.


  var doRead = state.needReadable;
  debug('need readable', doRead); // If we currently have less than the highWaterMark, then also read some.

  if (state.length === 0 || state.length - n < state.highWaterMark) {
    doRead = true;
    debug('length less than watermark', doRead);
  } // However, if we've ended, then there's no point, if we're already
  // reading, then it's unnecessary, if we're constructing we have to wait,
  // and if we're destroyed or errored, then it's not allowed,


  if (state.ended || state.reading || state.destroyed || state.errored || !state.constructed) {
    doRead = false;
    debug('reading, ended or constructing', doRead);
  } else if (doRead) {
    debug('do read');
    state.reading = true;
    state.sync = true; // If the length is currently zero, then we *need* a readable event.

    if (state.length === 0) state.needReadable = true; // Call internal read method

    try {
      var result = this._read(state.highWaterMark);

      if (result != null) {
        var then = result.then;

        if (typeof then === 'function') {
          then.call(result, nop, function (err) {
            errorOrDestroy(this, err);
          });
        }
      }
    } catch (err) {
      errorOrDestroy(this, err);
    }

    state.sync = false; // If _read pushed data synchronously, then `reading` will be false,
    // and we need to re-evaluate how much data we can return to the user.

    if (!state.reading) n = howMuchToRead(nOrig, state);
  }

  var ret;
  if (n > 0) ret = fromList(n, state);else ret = null;

  if (ret === null) {
    state.needReadable = state.length <= state.highWaterMark;
    n = 0;
  } else {
    state.length -= n;

    if (state.multiAwaitDrain) {
      state.awaitDrainWriters.clear();
    } else {
      state.awaitDrainWriters = null;
    }
  }

  if (state.length === 0) {
    // If we have nothing in the buffer, then we want to know
    // as soon as we *do* get something into the buffer.
    if (!state.ended) state.needReadable = true; // If we tried to read() past the EOF, then emit end on the next tick.

    if (nOrig !== n && state.ended) endReadable(this);
  }

  if (ret !== null) this.emit('data', ret);
  return ret;
};

function onEofChunk(stream, state) {
  debug('onEofChunk');
  if (state.ended) return;

  if (state.decoder) {
    var chunk = state.decoder.end();

    if (chunk && chunk.length) {
      state.buffer.push(chunk);
      state.length += state.objectMode ? 1 : chunk.length;
    }
  }

  state.ended = true;

  if (state.sync) {
    // If we are sync, wait until next tick to emit the data.
    // Otherwise we risk emitting data in the flow()
    // the readable code triggers during a read() call.
    emitReadable(stream);
  } else {
    // Emit 'readable' now to make sure it gets picked up.
    state.needReadable = false;
    state.emittedReadable = true; // We have to emit readable now that we are EOF. Modules
    // in the ecosystem (e.g. dicer) rely on this event being sync.

    emitReadable_(stream);
  }
} // Don't emit readable right away in sync mode, because this can trigger
// another read() call => stack overflow.  This way, it might trigger
// a nextTick recursion warning, but that's not so bad.


function emitReadable(stream) {
  var state = stream._readableState;
  debug('emitReadable', state.needReadable, state.emittedReadable);
  state.needReadable = false;

  if (!state.emittedReadable) {
    debug('emitReadable', state.flowing);
    state.emittedReadable = true;
    process.nextTick(emitReadable_, stream);
  }
}

function emitReadable_(stream) {
  var state = stream._readableState;
  debug('emitReadable_', state.destroyed, state.length, state.ended);

  if (!state.destroyed && !state.errored && (state.length || state.ended)) {
    stream.emit('readable');
    state.emittedReadable = false;
  } // The stream needs another readable event if:
  // 1. It is not flowing, as the flow mechanism will take
  //    care of it.
  // 2. It is not ended.
  // 3. It is below the highWaterMark, so we can schedule
  //    another readable later.


  state.needReadable = !state.flowing && !state.ended && state.length <= state.highWaterMark;
  flow(stream);
} // At this point, the user has presumably seen the 'readable' event,
// and called read() to consume some data.  that may have triggered
// in turn another _read(n) call, in which case reading = true if
// it's in progress.
// However, if we're not ended, or reading, and the length < hwm,
// then go ahead and try to read some more preemptively.


function maybeReadMore(stream, state) {
  if (!state.readingMore && state.constructed) {
    state.readingMore = true;
    process.nextTick(maybeReadMore_, stream, state);
  }
}

function maybeReadMore_(stream, state) {
  // Attempt to read more data if we should.
  //
  // The conditions for reading more data are (one of):
  // - Not enough data buffered (state.length < state.highWaterMark). The loop
  //   is responsible for filling the buffer with enough data if such data
  //   is available. If highWaterMark is 0 and we are not in the flowing mode
  //   we should _not_ attempt to buffer any extra data. We'll get more data
  //   when the stream consumer calls read() instead.
  // - No data in the buffer, and the stream is in flowing mode. In this mode
  //   the loop below is responsible for ensuring read() is called. Failing to
  //   call read here would abort the flow and there's no other mechanism for
  //   continuing the flow if the stream consumer has just subscribed to the
  //   'data' event.
  //
  // In addition to the above conditions to keep reading data, the following
  // conditions prevent the data from being read:
  // - The stream has ended (state.ended).
  // - There is already a pending 'read' operation (state.reading). This is a
  //   case where the stream has called the implementation defined _read()
  //   method, but they are processing the call asynchronously and have _not_
  //   called push() with new data. In this case we skip performing more
  //   read()s. The execution ends in this method again after the _read() ends
  //   up calling push() with more data.
  while (!state.reading && !state.ended && (state.length < state.highWaterMark || state.flowing && state.length === 0)) {
    var len = state.length;
    debug('maybeReadMore read 0');
    stream.read(0);
    if (len === state.length) // Didn't get any data, stop spinning.
      break;
  }

  state.readingMore = false;
} // Abstract method.  to be overridden in specific implementation classes.
// call cb(er, data) where data is <= n in length.
// for virtual (non-string, non-buffer) streams, "length" is somewhat
// arbitrary, and perhaps not very meaningful.


Readable.prototype._read = function (n) {
  throw new ERR_METHOD_NOT_IMPLEMENTED('_read()');
};

Readable.prototype.pipe = function (dest, pipeOpts) {
  var src = this;
  var state = this._readableState;

  if (state.pipes.length === 1) {
    if (!state.multiAwaitDrain) {
      state.multiAwaitDrain = true;
      state.awaitDrainWriters = new SafeSet(state.awaitDrainWriters ? [state.awaitDrainWriters] : []);
    }
  }

  state.pipes.push(dest);
  debug('pipe count=%d opts=%j', state.pipes.length, pipeOpts);
  var doEnd = (!pipeOpts || pipeOpts.end !== false) && dest !== process.stdout && dest !== process.stderr;
  var endFn = doEnd ? onend : unpipe;
  if (state.endEmitted) process.nextTick(endFn);else src.once('end', endFn);
  dest.on('unpipe', onunpipe);

  function onunpipe(readable, unpipeInfo) {
    debug('onunpipe');

    if (readable === src) {
      if (unpipeInfo && unpipeInfo.hasUnpiped === false) {
        unpipeInfo.hasUnpiped = true;
        cleanup();
      }
    }
  }

  function onend() {
    debug('onend');
    dest.end();
  }

  var ondrain;
  var cleanedUp = false;

  function cleanup() {
    debug('cleanup'); // Cleanup event handlers once the pipe is broken.

    dest.removeListener('close', onclose);
    dest.removeListener('finish', onfinish);

    if (ondrain) {
      dest.removeListener('drain', ondrain);
    }

    dest.removeListener('error', onerror);
    dest.removeListener('unpipe', onunpipe);
    src.removeListener('end', onend);
    src.removeListener('end', unpipe);
    src.removeListener('data', ondata);
    cleanedUp = true; // If the reader is waiting for a drain event from this
    // specific writer, then it would cause it to never start
    // flowing again.
    // So, if this is awaiting a drain, then we just call it now.
    // If we don't know, then assume that we are waiting for one.

    if (ondrain && state.awaitDrainWriters && (!dest._writableState || dest._writableState.needDrain)) ondrain();
  }

  function pause() {
    // If the user unpiped during `dest.write()`, it is possible
    // to get stuck in a permanently paused state if that write
    // also returned false.
    // => Check whether `dest` is still a piping destination.
    if (!cleanedUp) {
      if (state.pipes.length === 1 && state.pipes[0] === dest) {
        debug('false write response, pause', 0);
        state.awaitDrainWriters = dest;
        state.multiAwaitDrain = false;
      } else if (state.pipes.length > 1 && state.pipes.includes(dest)) {
        debug('false write response, pause', state.awaitDrainWriters.size);
        state.awaitDrainWriters.add(dest);
      }

      src.pause();
    }

    if (!ondrain) {
      // When the dest drains, it reduces the awaitDrain counter
      // on the source.  This would be more elegant with a .once()
      // handler in flow(), but adding and removing repeatedly is
      // too slow.
      ondrain = pipeOnDrain(src, dest);
      dest.on('drain', ondrain);
    }
  }

  src.on('data', ondata);

  function ondata(chunk) {
    debug('ondata');
    var ret = dest.write(chunk);
    debug('dest.write', ret);

    if (ret === false) {
      pause();
    }
  } // If the dest has an error, then stop piping into it.
  // However, don't suppress the throwing behavior for this.


  function onerror(er) {
    debug('onerror', er);
    unpipe();
    dest.removeListener('error', onerror);

    if (EE.listenerCount(dest, 'error') === 0) {
      var s = dest._writableState || dest._readableState;

      if (s && !s.errorEmitted) {
        // User incorrectly emitted 'error' directly on the stream.
        errorOrDestroy(dest, er);
      } else {
        dest.emit('error', er);
      }
    }
  } // Make sure our error handler is attached before userland ones.


  prependListener(dest, 'error', onerror); // Both close and finish should trigger unpipe, but only once.

  function onclose() {
    dest.removeListener('finish', onfinish);
    unpipe();
  }

  dest.once('close', onclose);

  function onfinish() {
    debug('onfinish');
    dest.removeListener('close', onclose);
    unpipe();
  }

  dest.once('finish', onfinish);

  function unpipe() {
    debug('unpipe');
    src.unpipe(dest);
  } // Tell the dest that it's being piped to.


  dest.emit('pipe', src); // Start the flow if it hasn't been started already.

  if (dest.writableNeedDrain === true) {
    if (state.flowing) {
      pause();
    }
  } else if (!state.flowing) {
    debug('pipe resume');
    src.resume();
  }

  return dest;
};

function pipeOnDrain(src, dest) {
  return function pipeOnDrainFunctionResult() {
    var state = src._readableState; // `ondrain` will call directly,
    // `this` maybe not a reference to dest,
    // so we use the real dest here.

    if (state.awaitDrainWriters === dest) {
      debug('pipeOnDrain', 1);
      state.awaitDrainWriters = null;
    } else if (state.multiAwaitDrain) {
      debug('pipeOnDrain', state.awaitDrainWriters.size);
      state.awaitDrainWriters["delete"](dest);
    }

    if ((!state.awaitDrainWriters || state.awaitDrainWriters.size === 0) && EE.listenerCount(src, 'data')) {
      state.flowing = true;
      flow(src);
    }
  };
}

Readable.prototype.unpipe = function (dest) {
  var state = this._readableState;
  var unpipeInfo = {
    hasUnpiped: false
  }; // If we're not piping anywhere, then do nothing.

  if (state.pipes.length === 0) return this;

  if (!dest) {
    // remove all.
    var dests = state.pipes;
    state.pipes = [];
    this.pause();

    for (var i = 0; i < dests.length; i++) {
      dests[i].emit('unpipe', this, {
        hasUnpiped: false
      });
    }

    return this;
  } // Try to find the right one.


  var index = ArrayPrototypeIndexOf(state.pipes, dest);
  if (index === -1) return this;
  state.pipes.splice(index, 1);
  if (state.pipes.length === 0) this.pause();
  dest.emit('unpipe', this, unpipeInfo);
  return this;
}; // Set up data events if they are asked for
// Ensure readable listeners eventually get something.


Readable.prototype.on = function (ev, fn) {
  var res = Stream.prototype.on.call(this, ev, fn);
  var state = this._readableState;

  if (ev === 'data') {
    // Update readableListening so that resume() may be a no-op
    // a few lines down. This is needed to support once('readable').
    state.readableListening = this.listenerCount('readable') > 0; // Try start flowing on next tick if stream isn't explicitly paused.

    if (state.flowing !== false) this.resume();
  } else if (ev === 'readable') {
    if (!state.endEmitted && !state.readableListening) {
      state.readableListening = state.needReadable = true;
      state.flowing = false;
      state.emittedReadable = false;
      debug('on readable', state.length, state.reading);

      if (state.length) {
        emitReadable(this);
      } else if (!state.reading) {
        process.nextTick(nReadingNextTick, this);
      }
    }
  }

  return res;
};

Readable.prototype.addListener = Readable.prototype.on;

Readable.prototype.removeListener = function (ev, fn) {
  var res = Stream.prototype.removeListener.call(this, ev, fn);

  if (ev === 'readable') {
    // We need to check if there is someone still listening to
    // readable and reset the state. However this needs to happen
    // after readable has been emitted but before I/O (nextTick) to
    // support once('readable', fn) cycles. This means that calling
    // resume within the same tick will have no
    // effect.
    process.nextTick(updateReadableListening, this);
  }

  return res;
};

Readable.prototype.off = Readable.prototype.removeListener;

Readable.prototype.removeAllListeners = function (ev) {
  var res = Stream.prototype.removeAllListeners.apply(this, arguments);

  if (ev === 'readable' || ev === undefined) {
    // We need to check if there is someone still listening to
    // readable and reset the state. However this needs to happen
    // after readable has been emitted but before I/O (nextTick) to
    // support once('readable', fn) cycles. This means that calling
    // resume within the same tick will have no
    // effect.
    process.nextTick(updateReadableListening, this);
  }

  return res;
};

function updateReadableListening(self) {
  var state = self._readableState;
  state.readableListening = self.listenerCount('readable') > 0;

  if (state.resumeScheduled && state[kPaused] === false) {
    // Flowing needs to be set to true now, otherwise
    // the upcoming resume will not flow.
    state.flowing = true; // Crude way to check if we should resume.
  } else if (self.listenerCount('data') > 0) {
    self.resume();
  } else if (!state.readableListening) {
    state.flowing = null;
  }
}

function nReadingNextTick(self) {
  debug('readable nexttick read 0');
  self.read(0);
} // pause() and resume() are remnants of the legacy readable stream API
// If the user uses them, then switch into old mode.


Readable.prototype.resume = function () {
  var state = this._readableState;

  if (!state.flowing) {
    debug('resume'); // We flow only if there is no one listening
    // for readable, but we still have to call
    // resume().

    state.flowing = !state.readableListening;
    resume(this, state);
  }

  state[kPaused] = false;
  return this;
};

function resume(stream, state) {
  if (!state.resumeScheduled) {
    state.resumeScheduled = true;
    process.nextTick(resume_, stream, state);
  }
}

function resume_(stream, state) {
  debug('resume', state.reading);

  if (!state.reading) {
    stream.read(0);
  }

  state.resumeScheduled = false;
  stream.emit('resume');
  flow(stream);
  if (state.flowing && !state.reading) stream.read(0);
}

Readable.prototype.pause = function () {
  debug('call pause flowing=%j', this._readableState.flowing);

  if (this._readableState.flowing !== false) {
    debug('pause');
    this._readableState.flowing = false;
    this.emit('pause');
  }

  this._readableState[kPaused] = true;
  return this;
};

function flow(stream) {
  var state = stream._readableState;
  debug('flow', state.flowing);

  while (state.flowing && stream.read() !== null) {
    ;
  }
} // Wrap an old-style stream as the async data source.
// This is *not* part of the readable stream interface.
// It is an ugly unfortunate mess of history.


Readable.prototype.wrap = function (stream) {
  var _this2 = this;

  var paused = false; // TODO (ronag): Should this.destroy(err) emit
  // 'error' on the wrapped stream? Would require
  // a static factory method, e.g. Readable.wrap(stream).

  stream.on('data', function (chunk) {
    if (!_this2.push(chunk) && stream.pause) {
      paused = true;
      stream.pause();
    }
  });
  stream.on('end', function () {
    _this2.push(null);
  });
  stream.on('error', function (err) {
    errorOrDestroy(_this2, err);
  });
  stream.on('close', function () {
    _this2.destroy();
  });
  stream.on('destroy', function () {
    _this2.destroy();
  });

  this._read = function () {
    if (paused && stream.resume) {
      paused = false;
      stream.resume();
    }
  }; // Proxy all the other methods. Important when wrapping filters and duplexes.


  var streamKeys = ObjectKeys(stream);

  for (var j = 1; j < streamKeys.length; j++) {
    var i = streamKeys[j];

    if (this[i] === undefined && typeof stream[i] === 'function') {
      this[i] = stream[i].bind(stream);
    }
  }

  return this;
};

Readable.prototype[SymbolAsyncIterator] = function () {
  return streamToAsyncIterator(this);
};

Readable.prototype.iterator = function (options) {
  if (options !== undefined) {
    validateObject(options, 'options');
  }

  return streamToAsyncIterator(this, options);
};

function streamToAsyncIterator(stream, options) {
  if (typeof stream.read !== 'function') {
    // v1 stream
    var src = stream;
    stream = new Readable({
      objectMode: true,
      destroy: function destroy(err, callback) {
        destroyImpl.destroyer(src, err);
        callback(err);
      }
    }).wrap(src);
  }

  var iter = createAsyncIterator(stream, options);
  iter.stream = stream;
  return iter;
}

function createAsyncIterator(_x, _x2) {
  return _createAsyncIterator.apply(this, arguments);
} // Making it explicit these properties are not enumerable
// because otherwise some prototype manipulation in
// userland will fail.


function _createAsyncIterator() {
  _createAsyncIterator = _wrapAsyncGenerator( /*#__PURE__*/regeneratorRuntime.mark(function _callee(stream, options) {
    var callback, next, error, chunk;
    return regeneratorRuntime.wrap(function _callee$(_context) {
      while (1) {
        switch (_context.prev = _context.next) {
          case 0:
            next = function _next(resolve) {
              if (this === stream) {
                callback();
                callback = nop;
              } else {
                callback = resolve;
              }
            };

            callback = nop;
            stream.on('readable', next);
            eos(stream, {
              writable: false
            }, function (err) {
              error = err ? aggregateTwoErrors(error, err) : null;
              callback();
              callback = nop;
            });
            _context.prev = 4;

          case 5:
            if (!true) {
              _context.next = 24;
              break;
            }

            chunk = stream.destroyed ? null : stream.read();

            if (!(chunk !== null)) {
              _context.next = 12;
              break;
            }

            _context.next = 10;
            return chunk;

          case 10:
            _context.next = 22;
            break;

          case 12:
            if (!error) {
              _context.next = 16;
              break;
            }

            throw error;

          case 16:
            if (!(error === null)) {
              _context.next = 20;
              break;
            }

            return _context.abrupt("return");

          case 20:
            _context.next = 22;
            return _awaitAsyncGenerator(new _Promise(next));

          case 22:
            _context.next = 5;
            break;

          case 24:
            _context.next = 30;
            break;

          case 26:
            _context.prev = 26;
            _context.t0 = _context["catch"](4);
            error = aggregateTwoErrors(error, _context.t0);
            throw error;

          case 30:
            _context.prev = 30;

            if ((error || (options === null || options === void 0 ? void 0 : options.destroyOnReturn) !== false) && (error === undefined || stream._readableState.autoDestroy)) {
              destroyImpl.destroyer(stream, null);
            }

            return _context.finish(30);

          case 33:
          case "end":
            return _context.stop();
        }
      }
    }, _callee, null, [[4, 26, 30, 33]]);
  }));
  return _createAsyncIterator.apply(this, arguments);
}

ObjectDefineProperties(Readable.prototype, {
  readable: {
    get: function get() {
      var r = this._readableState; // r.readable === false means that this is part of a Duplex stream
      // where the readable side was disabled upon construction.
      // Compat. The user might manually disable readable side through
      // deprecated setter.

      return !!r && r.readable !== false && !r.destroyed && !r.errorEmitted && !r.endEmitted;
    },
    set: function set(val) {
      // Backwards compat.
      if (this._readableState) {
        this._readableState.readable = !!val;
      }
    }
  },
  readableHighWaterMark: {
    enumerable: false,
    get: function get() {
      return this._readableState.highWaterMark;
    }
  },
  readableBuffer: {
    enumerable: false,
    get: function get() {
      return this._readableState && this._readableState.buffer;
    }
  },
  readableFlowing: {
    enumerable: false,
    get: function get() {
      return this._readableState.flowing;
    },
    set: function set(state) {
      if (this._readableState) {
        this._readableState.flowing = state;
      }
    }
  },
  readableLength: {
    enumerable: false,
    get: function get() {
      return this._readableState.length;
    }
  },
  readableObjectMode: {
    enumerable: false,
    get: function get() {
      return this._readableState ? this._readableState.objectMode : false;
    }
  },
  readableEncoding: {
    enumerable: false,
    get: function get() {
      return this._readableState ? this._readableState.encoding : null;
    }
  },
  destroyed: {
    enumerable: false,
    get: function get() {
      if (this._readableState === undefined) {
        return false;
      }

      return this._readableState.destroyed;
    },
    set: function set(value) {
      // We ignore the value if the stream
      // has not been initialized yet.
      if (!this._readableState) {
        return;
      } // Backward compatibility, the user is explicitly
      // managing destroyed.


      this._readableState.destroyed = value;
    }
  },
  readableEnded: {
    enumerable: false,
    get: function get() {
      return this._readableState ? this._readableState.endEmitted : false;
    }
  }
});
ObjectDefineProperties(ReadableState.prototype, {
  // Legacy getter for `pipesCount`.
  pipesCount: {
    get: function get() {
      return this.pipes.length;
    }
  },
  // Legacy property for `paused`.
  paused: {
    get: function get() {
      return this[kPaused] !== false;
    },
    set: function set(value) {
      this[kPaused] = !!value;
    }
  }
}); // Exposed for testing purposes only.

Readable._fromList = fromList; // Pluck off n bytes from an array of buffers.
// Length is the combined lengths of all the buffers in the list.
// This function is designed to be inlinable, so please take care when making
// changes to the function body.

function fromList(n, state) {
  // nothing buffered.
  if (state.length === 0) return null;
  var ret;
  if (state.objectMode) ret = state.buffer.shift();else if (!n || n >= state.length) {
    // Read it all, truncate the list.
    if (state.decoder) ret = state.buffer.join('');else if (state.buffer.length === 1) ret = state.buffer.first();else ret = state.buffer.concat(state.length);
    state.buffer.clear();
  } else {
    // read part of list.
    ret = state.buffer.consume(n, state.decoder);
  }
  return ret;
}

function endReadable(stream) {
  var state = stream._readableState;
  debug('endReadable', state.endEmitted);

  if (!state.endEmitted) {
    state.ended = true;
    process.nextTick(endReadableNT, state, stream);
  }
}

function endReadableNT(state, stream) {
  debug('endReadableNT', state.endEmitted, state.length); // Check that we didn't get one last unshift.

  if (!state.errorEmitted && !state.closeEmitted && !state.endEmitted && state.length === 0) {
    state.endEmitted = true;
    stream.emit('end');

    if (stream.writable && stream.allowHalfOpen === false) {
      process.nextTick(endWritableNT, state, stream);
    } else if (state.autoDestroy) {
      // In case of duplex streams we need a way to detect
      // if the writable side is ready for autoDestroy as well.
      var wState = stream._writableState;
      var autoDestroy = !wState || wState.autoDestroy && ( // We don't expect the writable to ever 'finish'
      // if writable is explicitly set to false.
      wState.finished || wState.writable === false);

      if (autoDestroy) {
        stream.destroy();
      }
    }
  }
}

function endWritableNT(state, stream) {
  var writable = stream.writable && !stream.writableEnded && !stream.destroyed;

  if (writable) {
    stream.end();
  }
}

Readable.from = function (iterable, opts) {
  return from(Readable, iterable, opts);
};
