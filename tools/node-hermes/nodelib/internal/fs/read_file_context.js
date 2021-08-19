// @nolint
'use strict';

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var _primordials = primordials,
    ArrayPrototypePush = _primordials.ArrayPrototypePush,
    MathMin = _primordials.MathMin,
    ReflectApply = _primordials.ReflectApply;

var _require = require('internal/fs/utils'),
    _require$constants = _require.constants,
    kReadFileBufferLength = _require$constants.kReadFileBufferLength,
    kReadFileUnknownBufferLength = _require$constants.kReadFileUnknownBufferLength;

var _require2 = require('buffer'),
    Buffer = _require2.Buffer;

var _internalBinding = internalBinding('fs'),
    FSReqCallback = _internalBinding.FSReqCallback,
    _close = _internalBinding.close,
    _read = _internalBinding.read;

var _require3 = require('internal/errors'),
    AbortError = _require3.AbortError,
    aggregateTwoErrors = _require3.aggregateTwoErrors;

function readFileAfterRead(err, bytesRead) {
  var context = this.context;
  if (err) return context.close(err);
  context.pos += bytesRead;

  if (context.pos === context.size || bytesRead === 0) {
    context.close();
  } else {
    if (context.size === 0) {
      // Unknown size, just read until we don't get bytes.
      var buffer = bytesRead === kReadFileUnknownBufferLength ? context.buffer : context.buffer.slice(0, bytesRead);
      ArrayPrototypePush(context.buffers, buffer);
    }

    context.read();
  }
}

function readFileAfterClose(err) {
  var context = this.context;
  var callback = context.callback;
  var buffer = null;
  if (context.err || err) return callback(aggregateTwoErrors(err, context.err));

  try {
    if (context.size === 0) buffer = Buffer.concat(context.buffers, context.pos);else if (context.pos < context.size) buffer = context.buffer.slice(0, context.pos);else buffer = context.buffer;
    if (context.encoding) buffer = buffer.toString(context.encoding);
  } catch (err) {
    return callback(err);
  }

  callback(null, buffer);
}

var ReadFileContext = /*#__PURE__*/function () {
  function ReadFileContext(callback, encoding) {
    _classCallCheck(this, ReadFileContext);

    this.fd = undefined;
    this.isUserFd = undefined;
    this.size = 0;
    this.callback = callback;
    this.buffers = null;
    this.buffer = null;
    this.pos = 0;
    this.encoding = encoding;
    this.err = null;
    this.signal = undefined;
  }

  _createClass(ReadFileContext, [{
    key: "read",
    value: function read() {
      var _this$signal;

      var buffer;
      var offset;
      var length;

      if ((_this$signal = this.signal) !== null && _this$signal !== void 0 && _this$signal.aborted) {
        return this.close(new AbortError());
      }

      if (this.size === 0) {
        buffer = Buffer.allocUnsafeSlow(kReadFileUnknownBufferLength);
        offset = 0;
        length = kReadFileUnknownBufferLength;
        this.buffer = buffer;
      } else {
        buffer = this.buffer;
        offset = this.pos;
        length = MathMin(kReadFileBufferLength, this.size - this.pos);
      }

      var req = new FSReqCallback();
      req.oncomplete = readFileAfterRead;
      req.context = this;

      _read(this.fd, buffer, offset, length, -1, req);
    }
  }, {
    key: "close",
    value: function close(err) {
      if (this.isUserFd) {
        process.nextTick(function tick(context) {
          ReflectApply(readFileAfterClose, {
            context: context
          }, [null]);
        }, this);
        return;
      }

      var req = new FSReqCallback();
      req.oncomplete = readFileAfterClose;
      req.context = this;
      this.err = err;

      _close(this.fd, req);
    }
  }]);

  return ReadFileContext;
}();

module.exports = ReadFileContext;
