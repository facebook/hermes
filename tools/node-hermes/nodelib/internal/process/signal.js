'use strict';

var _primordials = primordials,
    FunctionPrototypeBind = _primordials.FunctionPrototypeBind,
    SafeMap = _primordials.SafeMap;

var _require = require('internal/errors'),
    errnoException = _require.errnoException;

var signals = internalBinding('constants').os.signals;
var Signal;
var signalWraps = new SafeMap();

function isSignal(event) {
  return typeof event === 'string' && signals[event] !== undefined;
} // Detect presence of a listener for the special signal types


function startListeningIfSignal(type) {
  if (isSignal(type) && !signalWraps.has(type)) {
    if (Signal === undefined) Signal = internalBinding('signal_wrap').Signal;
    var wrap = new Signal();
    wrap.unref();
    wrap.onsignal = FunctionPrototypeBind(process.emit, process, type, type);
    var signum = signals[type];
    var err = wrap.start(signum);

    if (err) {
      wrap.close();
      throw errnoException(err, 'uv_signal_start');
    }

    signalWraps.set(type, wrap);
  }
}

function stopListeningIfSignal(type) {
  var wrap = signalWraps.get(type);

  if (wrap !== undefined && process.listenerCount(type) === 0) {
    wrap.close();
    signalWraps["delete"](type);
  }
}

module.exports = {
  startListeningIfSignal: startListeningIfSignal,
  stopListeningIfSignal: stopListeningIfSignal
};
