'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

var _require = require('internal/errors'),
    AbortError = _require.AbortError,
    codes = _require.codes;

var eos = require('internal/streams/end-of-stream');

var ERR_INVALID_ARG_TYPE = codes.ERR_INVALID_ARG_TYPE; // This method is inlined here for readable-stream
// It also does not allow for signal to not exist on the stream
// https://github.com/nodejs/node/pull/36061#discussion_r533718029

var validateAbortSignal = function validateAbortSignal(signal, name) {
  if (_typeof(signal) !== 'object' || !('aborted' in signal)) {
    throw new ERR_INVALID_ARG_TYPE(name, 'AbortSignal', signal);
  }
};

function isNodeStream(obj) {
  return !!(obj && typeof obj.pipe === 'function');
}

module.exports.addAbortSignal = function addAbortSignal(signal, stream) {
  validateAbortSignal(signal, 'signal');

  if (!isNodeStream(stream)) {
    throw new ERR_INVALID_ARG_TYPE('stream', 'stream.Stream', stream);
  }

  return module.exports.addAbortSignalNoValidate(signal, stream);
};

module.exports.addAbortSignalNoValidate = function (signal, stream) {
  if (_typeof(signal) !== 'object' || !('aborted' in signal)) {
    return stream;
  }

  var onAbort = function onAbort() {
    stream.destroy(new AbortError());
  };

  if (signal.aborted) {
    onAbort();
  } else {
    signal.addEventListener('abort', onAbort);
    eos(stream, function () {
      return signal.removeEventListener('abort', onAbort);
    });
  }

  return stream;
};
