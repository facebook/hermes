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

var _primordials = primordials,
    ObjectDefineProperty = _primordials.ObjectDefineProperty;

var _require = require('internal/util'),
    customPromisify = _require.promisify.custom;

var pipeline = require('internal/streams/pipeline');

var _require2 = require('internal/streams/destroy'),
    destroyer = _require2.destroyer;

var eos = require('internal/streams/end-of-stream');

var internalBuffer = require('internal/buffer');

var promises = require('stream/promises');

var Stream = module.exports = require('internal/streams/legacy').Stream;

Stream.Readable = require('internal/streams/readable');
Stream.Writable = require('internal/streams/writable');
Stream.Duplex = require('internal/streams/duplex');
Stream.Transform = require('internal/streams/transform');
Stream.PassThrough = require('internal/streams/passthrough');
Stream.pipeline = pipeline;

var _require3 = require('internal/streams/add-abort-signal'),
    addAbortSignal = _require3.addAbortSignal;

Stream.addAbortSignal = addAbortSignal;
Stream.finished = eos;
Stream.destroy = destroyer;
ObjectDefineProperty(Stream, 'promises', {
  configurable: true,
  enumerable: true,
  get: function get() {
    return promises;
  }
});
ObjectDefineProperty(pipeline, customPromisify, {
  enumerable: true,
  get: function get() {
    return promises.pipeline;
  }
});
ObjectDefineProperty(eos, customPromisify, {
  enumerable: true,
  get: function get() {
    return promises.finished;
  }
}); // Backwards-compat with node 0.4.x

Stream.Stream = Stream;
Stream._isUint8Array = require('internal/util/types').isUint8Array;

Stream._uint8ArrayToBuffer = function _uint8ArrayToBuffer(chunk) {
  return new internalBuffer.FastBuffer(chunk.buffer, chunk.byteOffset, chunk.byteLength);
};
