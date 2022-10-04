// Ported from https://github.com/mafintosh/end-of-stream with
// permission from the author, Mathias Buus (@mafintosh).
'use strict';

var _require = require('internal/errors'),
    AbortError = _require.AbortError,
    codes = _require.codes;

var ERR_STREAM_PREMATURE_CLOSE = codes.ERR_STREAM_PREMATURE_CLOSE;

var _require2 = require('internal/util'),
    once = _require2.once;

var _require3 = require('internal/validators'),
    validateAbortSignal = _require3.validateAbortSignal,
    validateFunction = _require3.validateFunction,
    validateObject = _require3.validateObject;

var _require4 = require('internal/streams/utils'),
    isClosed = _require4.isClosed,
    isReadable = _require4.isReadable,
    isReadableNodeStream = _require4.isReadableNodeStream,
    isReadableFinished = _require4.isReadableFinished,
    isWritable = _require4.isWritable,
    isWritableNodeStream = _require4.isWritableNodeStream,
    isWritableFinished = _require4.isWritableFinished,
    _willEmitClose = _require4.willEmitClose;

function isRequest(stream) {
  return stream.setHeader && typeof stream.abort === 'function';
}

var nop = function nop() {};

function eos(stream, options, callback) {
  if (arguments.length === 2) {
    callback = options;
    options = {};
  } else if (options == null) {
    options = {};
  } else {
    validateObject(options, 'options');
  }

  validateFunction(callback, 'callback');
  validateAbortSignal(options.signal, 'options.signal');
  callback = once(callback);
  var readable = options.readable || options.readable !== false && isReadableNodeStream(stream);
  var writable = options.writable || options.writable !== false && isWritableNodeStream(stream);
  var wState = stream._writableState;
  var rState = stream._readableState;

  var onlegacyfinish = function onlegacyfinish() {
    if (!stream.writable) onfinish();
  }; // TODO (ronag): Improve soft detection to include core modules and
  // common ecosystem modules that do properly emit 'close' but fail
  // this generic check.


  var willEmitClose = _willEmitClose(stream) && isReadableNodeStream(stream) === readable && isWritableNodeStream(stream) === writable;
  var writableFinished = isWritableFinished(stream, false);

  var onfinish = function onfinish() {
    writableFinished = true; // Stream should not be destroyed here. If it is that
    // means that user space is doing something differently and
    // we cannot trust willEmitClose.

    if (stream.destroyed) willEmitClose = false;
    if (willEmitClose && (!stream.readable || readable)) return;
    if (!readable || readableFinished) callback.call(stream);
  };

  var readableFinished = isReadableFinished(stream, false);

  var onend = function onend() {
    readableFinished = true; // Stream should not be destroyed here. If it is that
    // means that user space is doing something differently and
    // we cannot trust willEmitClose.

    if (stream.destroyed) willEmitClose = false;
    if (willEmitClose && (!stream.writable || writable)) return;
    if (!writable || writableFinished) callback.call(stream);
  };

  var onerror = function onerror(err) {
    callback.call(stream, err);
  };

  var closed = isClosed(stream);

  var onclose = function onclose() {
    closed = true;
    var errored = (wState === null || wState === void 0 ? void 0 : wState.errored) || (rState === null || rState === void 0 ? void 0 : rState.errored);

    if (errored && typeof errored !== 'boolean') {
      return callback.call(stream, errored);
    }

    if (readable && !readableFinished) {
      if (!isReadableFinished(stream, false)) return callback.call(stream, new ERR_STREAM_PREMATURE_CLOSE());
    }

    if (writable && !writableFinished) {
      if (!isWritableFinished(stream, false)) return callback.call(stream, new ERR_STREAM_PREMATURE_CLOSE());
    }

    callback.call(stream);
  };

  var onrequest = function onrequest() {
    stream.req.on('finish', onfinish);
  };

  if (isRequest(stream)) {
    stream.on('complete', onfinish);

    if (!willEmitClose) {
      stream.on('abort', onclose);
    }

    if (stream.req) onrequest();else stream.on('request', onrequest);
  } else if (writable && !wState) {
    // legacy streams
    stream.on('end', onlegacyfinish);
    stream.on('close', onlegacyfinish);
  } // Not all streams will emit 'close' after 'aborted'.


  if (!willEmitClose && typeof stream.aborted === 'boolean') {
    stream.on('aborted', onclose);
  }

  stream.on('end', onend);
  stream.on('finish', onfinish);
  if (options.error !== false) stream.on('error', onerror);
  stream.on('close', onclose);

  if (closed) {
    process.nextTick(onclose);
  } else if (wState !== null && wState !== void 0 && wState.errorEmitted || rState !== null && rState !== void 0 && rState.errorEmitted) {
    if (!willEmitClose) {
      process.nextTick(onclose);
    }
  } else if (!readable && (!willEmitClose || isReadable(stream)) && (writableFinished || !isWritable(stream))) {
    process.nextTick(onclose);
  } else if (!writable && (!willEmitClose || isWritable(stream)) && (readableFinished || !isReadable(stream))) {
    process.nextTick(onclose);
  } else if (rState && stream.req && stream.aborted) {
    process.nextTick(onclose);
  }

  var cleanup = function cleanup() {
    callback = nop;
    stream.removeListener('aborted', onclose);
    stream.removeListener('complete', onfinish);
    stream.removeListener('abort', onclose);
    stream.removeListener('request', onrequest);
    if (stream.req) stream.req.removeListener('finish', onfinish);
    stream.removeListener('end', onlegacyfinish);
    stream.removeListener('close', onlegacyfinish);
    stream.removeListener('finish', onfinish);
    stream.removeListener('end', onend);
    stream.removeListener('error', onerror);
    stream.removeListener('close', onclose);
  };

  if (options.signal && !closed) {
    var abort = function abort() {
      // Keep it because cleanup removes it.
      var endCallback = callback;
      cleanup();
      endCallback.call(stream, new AbortError());
    };

    if (options.signal.aborted) {
      process.nextTick(abort);
    } else {
      var originalCallback = callback;
      callback = once(function () {
        options.signal.removeEventListener('abort', abort);

        for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
          args[_key] = arguments[_key];
        }

        originalCallback.apply(stream, args);
      });
      options.signal.addEventListener('abort', abort);
    }
  }

  return cleanup;
}

module.exports = eos;
