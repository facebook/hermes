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
// a transform stream is a readable/writable stream where you do
// something with the data.  Sometimes it's called a "filter",
// but that's not a great name for it, since that implies a thing where
// some bits pass through, and others are simply ignored.  (That would
// be a valid example of a transform, of course.)
//
// While the output is causally related to the input, it's not a
// necessarily symmetric or synchronous transformation.  For example,
// a zlib stream might take multiple plain-text writes(), and then
// emit a single compressed chunk some time in the future.
//
// Here's how this works:
//
// The Transform stream has all the aspects of the readable and writable
// stream classes.  When you write(chunk), that calls _write(chunk,cb)
// internally, and returns false if there's a lot of pending writes
// buffered up.  When you call read(), that calls _read(n) until
// there's enough pending readable data buffered up.
//
// In a transform stream, the written data is placed in a buffer.  When
// _read(n) is called, it transforms the queued up data, calling the
// buffered _write cb's as it consumes chunks.  If consuming a single
// written chunk would result in multiple output chunks, then the first
// outputted bit calls the readcb, and subsequent chunks just go into
// the read buffer, and will cause it to emit 'readable' if necessary.
//
// This way, back-pressure is actually determined by the reading side,
// since _read has to be called to start processing a new chunk.  However,
// a pathological inflate type of transform can cause excessive buffering
// here.  For example, imagine a stream where every byte of input is
// interpreted as an integer from 0-255, and then results in that many
// bytes of output.  Writing the 4 bytes {ff,ff,ff,ff} would result in
// 1kb of data being output.  In this case, you could write a very small
// amount of input, and end up with a very large amount of output.  In
// such a pathological inflating mechanism, there'd be no way to tell
// the system to stop doing the transform.  A single 4MB write could
// cause the system to run out of memory.
//
// However, even in such a pathological case, only a single written chunk
// would be consumed, and then the rest would wait (un-transformed) until
// the results of the previous transformed chunk were consumed.
'use strict';

var _primordials = primordials,
    ObjectSetPrototypeOf = _primordials.ObjectSetPrototypeOf,
    _Symbol = _primordials.Symbol;
module.exports = Transform;

var ERR_METHOD_NOT_IMPLEMENTED = require('internal/errors').codes.ERR_METHOD_NOT_IMPLEMENTED;

var Duplex = require('internal/streams/duplex');

ObjectSetPrototypeOf(Transform.prototype, Duplex.prototype);
ObjectSetPrototypeOf(Transform, Duplex);

var kCallback = _Symbol('kCallback');

function Transform(options) {
  if (!(this instanceof Transform)) return new Transform(options);
  Duplex.call(this, options); // We have implemented the _read method, and done the other things
  // that Readable wants before the first _read call, so unset the
  // sync guard flag.

  this._readableState.sync = false;
  this[kCallback] = null;

  if (options) {
    if (typeof options.transform === 'function') this._transform = options.transform;
    if (typeof options.flush === 'function') this._flush = options.flush;
  } // When the writable side finishes, then flush out anything remaining.
  // Backwards compat. Some Transform streams incorrectly implement _final
  // instead of or in addition to _flush. By using 'prefinish' instead of
  // implementing _final we continue supporting this unfortunate use case.


  this.on('prefinish', prefinish);
}

function _final(cb) {
  var _this = this;

  var called = false;

  if (typeof this._flush === 'function' && !this.destroyed) {
    var result = this._flush(function (er, data) {
      called = true;

      if (er) {
        if (cb) {
          cb(er);
        } else {
          _this.destroy(er);
        }

        return;
      }

      if (data != null) {
        _this.push(data);
      }

      _this.push(null);

      if (cb) {
        cb();
      }
    });

    if (result !== undefined && result !== null) {
      try {
        var then = result.then;

        if (typeof then === 'function') {
          then.call(result, function (data) {
            if (called) return;
            if (data != null) _this.push(data);

            _this.push(null);

            if (cb) process.nextTick(cb);
          }, function (err) {
            if (cb) {
              process.nextTick(cb, err);
            } else {
              process.nextTick(function () {
                return _this.destroy(err);
              });
            }
          });
        }
      } catch (err) {
        process.nextTick(function () {
          return _this.destroy(err);
        });
      }
    }
  } else {
    this.push(null);

    if (cb) {
      cb();
    }
  }
}

function prefinish() {
  if (this._final !== _final) {
    _final.call(this);
  }
}

Transform.prototype._final = _final;

Transform.prototype._transform = function (chunk, encoding, callback) {
  throw new ERR_METHOD_NOT_IMPLEMENTED('_transform()');
};

Transform.prototype._write = function (chunk, encoding, callback) {
  var _this2 = this;

  var rState = this._readableState;
  var wState = this._writableState;
  var length = rState.length;
  var called = false;

  var result = this._transform(chunk, encoding, function (err, val) {
    called = true;

    if (err) {
      callback(err);
      return;
    }

    if (val != null) {
      _this2.push(val);
    }

    if (wState.ended || // Backwards compat.
    length === rState.length || // Backwards compat.
    rState.length < rState.highWaterMark || rState.length === 0) {
      callback();
    } else {
      _this2[kCallback] = callback;
    }
  });

  if (result !== undefined && result != null) {
    try {
      var then = result.then;

      if (typeof then === 'function') {
        then.call(result, function (val) {
          if (called) return;

          if (val != null) {
            _this2.push(val);
          }

          if (wState.ended || length === rState.length || rState.length < rState.highWaterMark || rState.length === 0) {
            process.nextTick(callback);
          } else {
            _this2[kCallback] = callback;
          }
        }, function (err) {
          process.nextTick(callback, err);
        });
      }
    } catch (err) {
      process.nextTick(callback, err);
    }
  }
};

Transform.prototype._read = function () {
  if (this[kCallback]) {
    var callback = this[kCallback];
    this[kCallback] = null;
    callback();
  }
};
