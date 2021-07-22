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
    MathTrunc = _primordials.MathTrunc,
    ObjectCreate = _primordials.ObjectCreate,
    ObjectDefineProperty = _primordials.ObjectDefineProperty,
    SymbolToPrimitive = _primordials.SymbolToPrimitive;

var _internalBinding = internalBinding('timers'),
    immediateInfo = _internalBinding.immediateInfo,
    toggleImmediateRef = _internalBinding.toggleImmediateRef;

var L = require('internal/linkedlist');

var _require = require('internal/timers'),
    async_id_symbol = _require.async_id_symbol,
    Timeout = _require.Timeout,
    Immediate = _require.Immediate,
    decRefCount = _require.decRefCount,
    _require$immediateInf = _require.immediateInfoFields,
    kCount = _require$immediateInf.kCount,
    kRefCount = _require$immediateInf.kRefCount,
    kRefed = _require.kRefed,
    kHasPrimitive = _require.kHasPrimitive,
    getTimerDuration = _require.getTimerDuration,
    timerListMap = _require.timerListMap,
    timerListQueue = _require.timerListQueue,
    immediateQueue = _require.immediateQueue,
    active = _require.active,
    unrefActive = _require.unrefActive,
    insert = _require.insert;

var _require2 = require('internal/util'),
    customPromisify = _require2.promisify.custom,
    deprecate = _require2.deprecate;

var debug = require('internal/util/debuglog').debuglog('timer', function (fn) {
  debug = fn;
});

var _require3 = require('internal/validators'),
    validateCallback = _require3.validateCallback;

var timersPromises;

var _require4 = require('internal/async_hooks'),
    destroyHooksExist = _require4.destroyHooksExist,
    emitDestroy = _require4.emitDestroy; // This stores all the known timer async ids to allow users to clearTimeout and
// clearInterval using those ids, to match the spec and the rest of the web
// platform.


var knownTimersById = ObjectCreate(null); // Remove a timer. Cancels the timeout and resets the relevant timer properties.

function unenroll(item) {
  if (item._destroyed) return;
  item._destroyed = true;
  if (item[kHasPrimitive]) delete knownTimersById[item[async_id_symbol]]; // Fewer checks may be possible, but these cover everything.

  if (destroyHooksExist() && item[async_id_symbol] !== undefined) emitDestroy(item[async_id_symbol]);
  L.remove(item); // We only delete refed lists because unrefed ones are incredibly likely
  // to come from http and be recreated shortly after.
  // TODO: Long-term this could instead be handled by creating an internal
  // clearTimeout that makes it clear that the list should not be deleted.
  // That function could then be used by http and other similar modules.

  if (item[kRefed]) {
    // Compliment truncation during insert().
    var msecs = MathTrunc(item._idleTimeout);
    var list = timerListMap[msecs];

    if (list !== undefined && L.isEmpty(list)) {
      debug('unenroll: list empty');
      timerListQueue.removeAt(list.priorityQueuePosition);
      delete timerListMap[list.msecs];
    }

    decRefCount();
  } // If active is called later, then we want to make sure not to insert again


  item._idleTimeout = -1;
} // Make a regular object able to act as a timer by setting some properties.
// This function does not start the timer, see `active()`.
// Using existing objects as timers slightly reduces object overhead.


function enroll(item, msecs) {
  msecs = getTimerDuration(msecs, 'msecs'); // If this item was already in a list somewhere
  // then we should unenroll it from that

  if (item._idleNext) unenroll(item);
  L.init(item);
  item._idleTimeout = msecs;
}
/*
 * DOM-style timers
 */


function setTimeout(callback, after, arg1, arg2, arg3) {
  validateCallback(callback);
  var i, args;

  switch (arguments.length) {
    // fast cases
    case 1:
    case 2:
      break;

    case 3:
      args = [arg1];
      break;

    case 4:
      args = [arg1, arg2];
      break;

    default:
      args = [arg1, arg2, arg3];

      for (i = 5; i < arguments.length; i++) {
        // Extend array dynamically, makes .apply run much faster in v6.0.0
        args[i - 2] = arguments[i];
      }

      break;
  }

  var timeout = new Timeout(callback, after, args, false, true);
  insert(timeout, timeout._idleTimeout);
  return timeout;
}

ObjectDefineProperty(setTimeout, customPromisify, {
  enumerable: true,
  get: function get() {
    if (!timersPromises) timersPromises = require('timers/promises');
    return timersPromises.setTimeout;
  }
});

function clearTimeout(timer) {
  if (timer && timer._onTimeout) {
    timer._onTimeout = null;
    unenroll(timer);
    return;
  }

  if (typeof timer === 'number' || typeof timer === 'string') {
    var timerInstance = knownTimersById[timer];

    if (timerInstance !== undefined) {
      timerInstance._onTimeout = null;
      unenroll(timerInstance);
    }
  }
}

function setInterval(callback, repeat, arg1, arg2, arg3) {
  validateCallback(callback);
  var i, args;

  switch (arguments.length) {
    // fast cases
    case 1:
    case 2:
      break;

    case 3:
      args = [arg1];
      break;

    case 4:
      args = [arg1, arg2];
      break;

    default:
      args = [arg1, arg2, arg3];

      for (i = 5; i < arguments.length; i++) {
        // Extend array dynamically, makes .apply run much faster in v6.0.0
        args[i - 2] = arguments[i];
      }

      break;
  }

  var timeout = new Timeout(callback, repeat, args, true, true);
  insert(timeout, timeout._idleTimeout);
  return timeout;
}

function clearInterval(timer) {
  // clearTimeout and clearInterval can be used to clear timers created from
  // both setTimeout and setInterval, as specified by HTML Living Standard:
  // https://html.spec.whatwg.org/multipage/timers-and-user-prompts.html#dom-setinterval
  clearTimeout(timer);
}

Timeout.prototype.close = function () {
  clearTimeout(this);
  return this;
};

Timeout.prototype[SymbolToPrimitive] = function () {
  var id = this[async_id_symbol];

  if (!this[kHasPrimitive]) {
    this[kHasPrimitive] = true;
    knownTimersById[id] = this;
  }

  return id;
};

function setImmediate(callback, arg1, arg2, arg3) {
  validateCallback(callback);
  var i, args;

  switch (arguments.length) {
    // fast cases
    case 1:
      break;

    case 2:
      args = [arg1];
      break;

    case 3:
      args = [arg1, arg2];
      break;

    default:
      args = [arg1, arg2, arg3];

      for (i = 4; i < arguments.length; i++) {
        // Extend array dynamically, makes .apply run much faster in v6.0.0
        args[i - 1] = arguments[i];
      }

      break;
  }

  return new Immediate(callback, args);
}

ObjectDefineProperty(setImmediate, customPromisify, {
  enumerable: true,
  get: function get() {
    if (!timersPromises) timersPromises = require('timers/promises');
    return timersPromises.setImmediate;
  }
});

function clearImmediate(immediate) {
  if (!immediate || immediate._destroyed) return;
  immediateInfo[kCount]--;
  immediate._destroyed = true;
  if (immediate[kRefed] && --immediateInfo[kRefCount] === 0) toggleImmediateRef(false);
  immediate[kRefed] = null;

  if (destroyHooksExist() && immediate[async_id_symbol] !== undefined) {
    emitDestroy(immediate[async_id_symbol]);
  }

  immediate._onImmediate = null;
  immediateQueue.remove(immediate);
}

module.exports = {
  setTimeout: setTimeout,
  clearTimeout: clearTimeout,
  setImmediate: setImmediate,
  clearImmediate: clearImmediate,
  setInterval: setInterval,
  clearInterval: clearInterval,
  _unrefActive: deprecate(unrefActive, 'timers._unrefActive() is deprecated.' + ' Please use timeout.refresh() instead.', 'DEP0127'),
  active: deprecate(active, 'timers.active() is deprecated. Please use timeout.refresh() instead.', 'DEP0126'),
  unenroll: deprecate(unenroll, 'timers.unenroll() is deprecated. Please use clearTimeout instead.', 'DEP0096'),
  enroll: deprecate(enroll, 'timers.enroll() is deprecated. Please use setTimeout instead.', 'DEP0095')
};
