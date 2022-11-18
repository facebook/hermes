'use strict';

var _primordials = primordials,
    _Symbol = _primordials.Symbol,
    SymbolAsyncIterator = _primordials.SymbolAsyncIterator,
    SymbolIterator = _primordials.SymbolIterator;

var kDestroyed = _Symbol('kDestroyed');

function isReadableNodeStream(obj) {
  var _obj$_readableState;

  return !!(obj && typeof obj.pipe === 'function' && typeof obj.on === 'function' && (!obj._writableState || ((_obj$_readableState = obj._readableState) === null || _obj$_readableState === void 0 ? void 0 : _obj$_readableState.readable) !== false) && ( // Duplex
  !obj._writableState || obj._readableState) // Writable has .pipe.
  );
}

function isWritableNodeStream(obj) {
  var _obj$_writableState;

  return !!(obj && typeof obj.write === 'function' && typeof obj.on === 'function' && (!obj._readableState || ((_obj$_writableState = obj._writableState) === null || _obj$_writableState === void 0 ? void 0 : _obj$_writableState.writable) !== false) // Duplex
  );
}

function isNodeStream(obj) {
  return isReadableNodeStream(obj) || isWritableNodeStream(obj);
}

function isIterable(obj, isAsync) {
  if (obj == null) return false;
  if (isAsync === true) return typeof obj[SymbolAsyncIterator] === 'function';
  if (isAsync === false) return typeof obj[SymbolIterator] === 'function';
  return typeof obj[SymbolAsyncIterator] === 'function' || typeof obj[SymbolIterator] === 'function';
}

function isDestroyed(stream) {
  if (!isNodeStream(stream)) return null;
  var wState = stream._writableState;
  var rState = stream._readableState;
  var state = wState || rState;
  return !!(stream.destroyed || stream[kDestroyed] || state !== null && state !== void 0 && state.destroyed);
} // Have been end():d.


function isWritableEnded(stream) {
  if (!isWritableNodeStream(stream)) return null;
  if (stream.writableEnded === true) return true;
  var wState = stream._writableState;
  if (wState !== null && wState !== void 0 && wState.errored) return false;
  if (typeof (wState === null || wState === void 0 ? void 0 : wState.ended) !== 'boolean') return null;
  return wState.ended;
} // Have emitted 'finish'.


function isWritableFinished(stream, strict) {
  if (!isWritableNodeStream(stream)) return null;
  if (stream.writableFinished === true) return true;
  var wState = stream._writableState;
  if (wState !== null && wState !== void 0 && wState.errored) return false;
  if (typeof (wState === null || wState === void 0 ? void 0 : wState.finished) !== 'boolean') return null;
  return !!(wState.finished || strict === false && wState.ended === true && wState.length === 0);
} // Have been push(null):d.


function isReadableEnded(stream) {
  if (!isReadableNodeStream(stream)) return null;
  if (stream.readableEnded === true) return true;
  var rState = stream._readableState;
  if (!rState || rState.errored) return false;
  if (typeof (rState === null || rState === void 0 ? void 0 : rState.ended) !== 'boolean') return null;
  return rState.ended;
} // Have emitted 'end'.


function isReadableFinished(stream, strict) {
  if (!isReadableNodeStream(stream)) return null;
  var rState = stream._readableState;
  if (rState !== null && rState !== void 0 && rState.errored) return false;
  if (typeof (rState === null || rState === void 0 ? void 0 : rState.endEmitted) !== 'boolean') return null;
  return !!(rState.endEmitted || strict === false && rState.ended === true && rState.length === 0);
}

function isReadable(stream) {
  var r = isReadableNodeStream(stream);
  if (r === null || typeof stream.readable !== 'boolean') return null;
  if (isDestroyed(stream)) return false;
  return r && stream.readable && !isReadableFinished(stream);
}

function isWritable(stream) {
  var r = isWritableNodeStream(stream);
  if (r === null || typeof stream.writable !== 'boolean') return null;
  if (isDestroyed(stream)) return false;
  return r && stream.writable && !isWritableEnded(stream);
}

function isFinished(stream, opts) {
  if (!isNodeStream(stream)) {
    return null;
  }

  if (isDestroyed(stream)) {
    return true;
  }

  if ((opts === null || opts === void 0 ? void 0 : opts.readable) !== false && isReadable(stream)) {
    return false;
  }

  if ((opts === null || opts === void 0 ? void 0 : opts.writable) !== false && isWritable(stream)) {
    return false;
  }

  return true;
}

function isClosed(stream) {
  if (!isNodeStream(stream)) {
    return null;
  }

  var wState = stream._writableState;
  var rState = stream._readableState;

  if (typeof (wState === null || wState === void 0 ? void 0 : wState.closed) === 'boolean' || typeof (rState === null || rState === void 0 ? void 0 : rState.closed) === 'boolean') {
    return (wState === null || wState === void 0 ? void 0 : wState.closed) || (rState === null || rState === void 0 ? void 0 : rState.closed);
  }

  if (typeof stream._closed === 'boolean' && isOutgoingMessage(stream)) {
    return stream._closed;
  }

  return null;
}

function isOutgoingMessage(stream) {
  return typeof stream._closed === 'boolean' && typeof stream._defaultKeepAlive === 'boolean' && typeof stream._removedConnection === 'boolean' && typeof stream._removedContLen === 'boolean';
}

function isServerResponse(stream) {
  return typeof stream._sent100 === 'boolean' && isOutgoingMessage(stream);
}

function isServerRequest(stream) {
  var _stream$req;

  return typeof stream._consuming === 'boolean' && typeof stream._dumped === 'boolean' && ((_stream$req = stream.req) === null || _stream$req === void 0 ? void 0 : _stream$req.upgradeOrConnect) === undefined;
}

function willEmitClose(stream) {
  if (!isNodeStream(stream)) return null;
  var wState = stream._writableState;
  var rState = stream._readableState;
  var state = wState || rState;
  return !state && isServerResponse(stream) || !!(state && state.autoDestroy && state.emitClose && state.closed === false);
}

module.exports = {
  kDestroyed: kDestroyed,
  isClosed: isClosed,
  isDestroyed: isDestroyed,
  isFinished: isFinished,
  isIterable: isIterable,
  isReadable: isReadable,
  isReadableNodeStream: isReadableNodeStream,
  isReadableEnded: isReadableEnded,
  isReadableFinished: isReadableFinished,
  isNodeStream: isNodeStream,
  isWritable: isWritable,
  isWritableNodeStream: isWritableNodeStream,
  isWritableEnded: isWritableEnded,
  isWritableFinished: isWritableFinished,
  isServerRequest: isServerRequest,
  isServerResponse: isServerResponse,
  willEmitClose: willEmitClose
};
