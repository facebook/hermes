'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var _primordials = primordials,
    ArrayPrototypePop = _primordials.ArrayPrototypePop,
    Promise = _primordials.Promise;

var _require = require('internal/streams/add-abort-signal'),
    addAbortSignalNoValidate = _require.addAbortSignalNoValidate;

var _require2 = require('internal/validators'),
    validateAbortSignal = _require2.validateAbortSignal;

var _require3 = require('internal/streams/utils'),
    isIterable = _require3.isIterable,
    isNodeStream = _require3.isNodeStream;

var pl = require('internal/streams/pipeline');
var eos = require('internal/streams/end-of-stream');

function pipeline() {
  for (var _len = arguments.length, streams = new Array(_len), _key = 0; _key < _len; _key++) {
    streams[_key] = arguments[_key];
  }

  return new Promise(function (resolve, reject) {
    var signal;
    var lastArg = streams[streams.length - 1];

    if (lastArg && _typeof(lastArg) === 'object' && !isNodeStream(lastArg) && !isIterable(lastArg)) {
      var options = ArrayPrototypePop(streams);
      signal = options.signal;
      validateAbortSignal(signal, 'options.signal');
    }

    var pipe = pl.apply(void 0, streams.concat([function (err, value) {
      if (err) {
        reject(err);
      } else {
        resolve(value);
      }
    }]));

    if (signal) {
      addAbortSignalNoValidate(signal, pipe);
    }
  });
}

function finished(stream, opts) {
  return new Promise(function (resolve, reject) {
    eos(stream, opts, function (err) {
      if (err) {
        reject(err);
      } else {
        resolve();
      }
    });
  });
}

module.exports = {
  finished: finished,
  pipeline: pipeline
};
