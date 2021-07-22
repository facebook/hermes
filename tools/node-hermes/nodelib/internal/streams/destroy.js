'use strict';

var _require = require('internal/errors'),
    aggregateTwoErrors = _require.aggregateTwoErrors,
    ERR_MULTIPLE_CALLBACK = _require.codes.ERR_MULTIPLE_CALLBACK,
    AbortError = _require.AbortError;

var _primordials = primordials,
    _Symbol = _primordials.Symbol;

var _require2 = require('internal/streams/utils'),
    kDestroyed = _require2.kDestroyed,
    isDestroyed = _require2.isDestroyed,
    isFinished = _require2.isFinished,
    isServerRequest = _require2.isServerRequest;

var kDestroy = _Symbol('kDestroy');

var kConstruct = _Symbol('kConstruct');

function checkError(err, w, r) {
  if (err) {
    // Avoid V8 leak, https://github.com/nodejs/node/pull/34103#issuecomment-652002364
    err.stack; // eslint-disable-line no-unused-expressions

    if (w && !w.errored) {
      w.errored = err;
    }

    if (r && !r.errored) {
      r.errored = err;
    }
  }
} // Backwards compat. cb() is undocumented and unused in core but
// unfortunately might be used by modules.


function destroy(err, cb) {
  var r = this._readableState;
  var w = this._writableState; // With duplex streams we use the writable side for state.

  var s = w || r;

  if (w && w.destroyed || r && r.destroyed) {
    if (typeof cb === 'function') {
      cb();
    }

    return this;
  } // We set destroyed to true before firing error callbacks in order
  // to make it re-entrance safe in case destroy() is called within callbacks


  checkError(err, w, r);

  if (w) {
    w.destroyed = true;
  }

  if (r) {
    r.destroyed = true;
  } // If still constructing then defer calling _destroy.


  if (!s.constructed) {
    this.once(kDestroy, function (er) {
      _destroy(this, aggregateTwoErrors(er, err), cb);
    });
  } else {
    _destroy(this, err, cb);
  }

  return this;
}


function _destroy(self, err, cb) {
  var called = false;

  function onDestroy(err) {
    if (called) {
      return;
    }

    called = true;
    var r = self._readableState;
    var w = self._writableState;
    checkError(err, w, r);

    if (w) {
      w.closed = true;
    }

    if (r) {
      r.closed = true;
    }

    if (typeof cb === 'function') {
      cb(err);
    }

    if (err) {
      process.nextTick(emitErrorCloseNT, self, err);
    } else {
      process.nextTick(emitCloseNT, self);
    }
  }

  try {
    var result = self._destroy(err || null, onDestroy);

    if (result != null) {
      var then = result.then;

      if (typeof then === 'function') {
        then.call(result, function () {
          process.nextTick(onDestroy, null);
        }, function (err) {
          process.nextTick(onDestroy, err);
        });
      }
    }
  } catch (err) {
    onDestroy(err);
  }
}

function emitErrorCloseNT(self, err) {
  emitErrorNT(self, err);
  emitCloseNT(self);
}

function emitCloseNT(self) {
  var r = self._readableState;
  var w = self._writableState;

  if (w) {
    w.closeEmitted = true;
  }

  if (r) {
    r.closeEmitted = true;
  }

  if (w && w.emitClose || r && r.emitClose) {
    self.emit('close');
  }
}

function emitErrorNT(self, err) {
  var r = self._readableState;
  var w = self._writableState;

  if (w && w.errorEmitted || r && r.errorEmitted) {
    return;
  }

  if (w) {
    w.errorEmitted = true;
  }

  if (r) {
    r.errorEmitted = true;
  }

  self.emit('error', err);
}

function undestroy() {
  var r = this._readableState;
  var w = this._writableState;

  if (r) {
    r.constructed = true;
    r.closed = false;
    r.closeEmitted = false;
    r.destroyed = false;
    r.errored = null;
    r.errorEmitted = false;
    r.reading = false;
    r.ended = r.readable === false;
    r.endEmitted = r.readable === false;
  }

  if (w) {
    w.constructed = true;
    w.destroyed = false;
    w.closed = false;
    w.closeEmitted = false;
    w.errored = null;
    w.errorEmitted = false;
    w.finalCalled = false;
    w.prefinished = false;
    w.ended = w.writable === false;
    w.ending = w.writable === false;
    w.finished = w.writable === false;
  }
}

function errorOrDestroy(stream, err, sync) {
  // We have tests that rely on errors being emitted
  // in the same tick, so changing this is semver major.
  // For now when you opt-in to autoDestroy we allow
  // the error to be emitted nextTick. In a future
  // semver major update we should change the default to this.
  var r = stream._readableState;
  var w = stream._writableState;

  if (w && w.destroyed || r && r.destroyed) {
    return this;
  }

  if (r && r.autoDestroy || w && w.autoDestroy) stream.destroy(err);else if (err) {
    // Avoid V8 leak, https://github.com/nodejs/node/pull/34103#issuecomment-652002364
    err.stack; // eslint-disable-line no-unused-expressions

    if (w && !w.errored) {
      w.errored = err;
    }

    if (r && !r.errored) {
      r.errored = err;
    }

    if (sync) {
      process.nextTick(emitErrorNT, stream, err);
    } else {
      emitErrorNT(stream, err);
    }
  }
}

function construct(stream, cb) {
  if (typeof stream._construct !== 'function') {
    return;
  }

  var r = stream._readableState;
  var w = stream._writableState;

  if (r) {
    r.constructed = false;
  }

  if (w) {
    w.constructed = false;
  }

  stream.once(kConstruct, cb);

  if (stream.listenerCount(kConstruct) > 1) {
    // Duplex
    return;
  }

  process.nextTick(constructNT, stream);
}


function constructNT(stream) {
  var called = false;

  function onConstruct(err) {
    if (called) {
      errorOrDestroy(stream, err !== null && err !== void 0 ? err : new ERR_MULTIPLE_CALLBACK());
      return;
    }

    called = true;
    var r = stream._readableState;
    var w = stream._writableState;
    var s = w || r;

    if (r) {
      r.constructed = true;
    }

    if (w) {
      w.constructed = true;
    }

    if (s.destroyed) {
      stream.emit(kDestroy, err);
    } else if (err) {
      errorOrDestroy(stream, err, true);
    } else {
      process.nextTick(emitConstructNT, stream);
    }
  }

  try {
    var result = stream._construct(onConstruct);

    if (result != null) {
      var then = result.then;

      if (typeof then === 'function') {
        then.call(result, function () {
          process.nextTick(onConstruct, null);
        }, function (err) {
          process.nextTick(onConstruct, err);
        });
      }
    }
  } catch (err) {
    onConstruct(err);
  }
}

function emitConstructNT(stream) {
  stream.emit(kConstruct);
}

function isRequest(stream) {
  return stream && stream.setHeader && typeof stream.abort === 'function';
}

function emitCloseLegacy(stream) {
  stream.emit('close');
}

function emitErrorCloseLegacy(stream, err) {
  stream.emit('error', err);
  process.nextTick(emitCloseLegacy, stream);
} // Normalize destroy for legacy.


function destroyer(stream, err) {
  if (isDestroyed(stream)) {
    return;
  }

  if (!err && !isFinished(stream)) {
    err = new AbortError();
  } // TODO: Remove isRequest branches.


  if (isServerRequest(stream)) {
    stream.socket = null;
    stream.destroy(err);
  } else if (isRequest(stream)) {
    stream.abort();
  } else if (isRequest(stream.req)) {
    stream.req.abort();
  } else if (typeof stream.destroy === 'function') {
    stream.destroy(err);
  } else if (typeof stream.close === 'function') {
    // TODO: Don't lose err?
    stream.close();
  } else if (err) {
    process.nextTick(emitErrorCloseLegacy, stream);
  } else {
    process.nextTick(emitCloseLegacy, stream);
  }

  if (!stream.destroyed) {
    stream[kDestroyed] = true;
  }
}

module.exports = {
  construct: construct,
  destroyer: destroyer,
  destroy: destroy,
  undestroy: undestroy,
  errorOrDestroy: errorOrDestroy
};
