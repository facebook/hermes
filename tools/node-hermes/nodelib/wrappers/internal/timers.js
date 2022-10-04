'use strict'; // HOW and WHY the timers implementation works the way it does.
//
// Timers are crucial to Node.js. Internally, any TCP I/O connection creates a
// timer so that we can time out of connections. Additionally, many user
// libraries and applications also use timers. As such there may be a
// significantly large amount of timeouts scheduled at any given time.
// Therefore, it is very important that the timers implementation is performant
// and efficient.
//
// Note: It is suggested you first read through the lib/internal/linkedlist.js
// linked list implementation, since timers depend on it extensively. It can be
// somewhat counter-intuitive at first, as it is not actually a class. Instead,
// it is a set of helpers that operate on an existing object.
//
// In order to be as performant as possible, the architecture and data
// structures are designed so that they are optimized to handle the following
// use cases as efficiently as possible:
// - Adding a new timer. (insert)
// - Removing an existing timer. (remove)
// - Handling a timer timing out. (timeout)
//
// Whenever possible, the implementation tries to make the complexity of these
// operations as close to constant-time as possible.
// (So that performance is not impacted by the number of scheduled timers.)
//
// Object maps are kept which contain linked lists keyed by their duration in
// milliseconds.
//

/* eslint-disable node-core/non-ascii-character */
//
// ╔════ > Object Map
// ║
// ╠══
// ║ lists: { '40': { }, '320': { etc } } (keys of millisecond duration)
// ╚══          ┌────┘
//              │
// ╔══          │
// ║ TimersList { _idleNext: { }, _idlePrev: (self) }
// ║         ┌────────────────┘
// ║    ╔══  │                              ^
// ║    ║    { _idleNext: { },  _idlePrev: { }, _onTimeout: (callback) }
// ║    ║      ┌───────────┘
// ║    ║      │                                  ^
// ║    ║      { _idleNext: { etc },  _idlePrev: { }, _onTimeout: (callback) }
// ╠══  ╠══
// ║    ║
// ║    ╚════ >  Actual JavaScript timeouts
// ║
// ╚════ > Linked List
//

/* eslint-enable node-core/non-ascii-character */
//
// With this, virtually constant-time insertion (append), removal, and timeout
// is possible in the JavaScript layer. Any one list of timers is able to be
// sorted by just appending to it because all timers within share the same
// duration. Therefore, any timer added later will always have been scheduled to
// timeout later, thus only needing to be appended.
// Removal from an object-property linked list is also virtually constant-time
// as can be seen in the lib/internal/linkedlist.js implementation.
// Timeouts only need to process any timers currently due to expire, which will
// always be at the beginning of the list for reasons stated above. Any timers
// after the first one encountered that does not yet need to timeout will also
// always be due to timeout at a later time.
//
// Less-than constant time operations are thus contained in two places:
// The PriorityQueue — an efficient binary heap implementation that does all
// operations in worst-case O(log n) time — which manages the order of expiring
// Timeout lists and the object map lookup of a specific list by the duration of
// timers within (or creation of a new list). However, these operations combined
// have shown to be trivial in comparison to other timers architectures.

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _toConsumableArray(arr) { return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread(); }

function _nonIterableSpread() { throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _iterableToArray(iter) { if (typeof Symbol !== "undefined" && iter[Symbol.iterator] != null || iter["@@iterator"] != null) return Array.from(iter); }

function _arrayWithoutHoles(arr) { if (Array.isArray(arr)) return _arrayLikeToArray(arr); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) { symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); } keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

var _primordials = primordials,
    MathMax = _primordials.MathMax,
    MathTrunc = _primordials.MathTrunc,
    NumberIsFinite = _primordials.NumberIsFinite,
    NumberMIN_SAFE_INTEGER = _primordials.NumberMIN_SAFE_INTEGER,
    ObjectCreate = _primordials.ObjectCreate,
    ReflectApply = _primordials.ReflectApply,
    _Symbol = _primordials.Symbol;

// var _internalBinding = internalBinding('timers'),
//     scheduleTimer = _internalBinding.scheduleTimer,
//     toggleTimerRef = _internalBinding.toggleTimerRef,
//     getLibuvNow = _internalBinding.getLibuvNow,
//     immediateInfo = _internalBinding.immediateInfo,
//     toggleImmediateRef = _internalBinding.toggleImmediateRef;

// var _require = require('internal/async_hooks'),
//     getDefaultTriggerAsyncId = _require.getDefaultTriggerAsyncId,
//     newAsyncId = _require.newAsyncId,
//     initHooksExist = _require.initHooksExist,
//     destroyHooksExist = _require.destroyHooksExist,
//     emitInit = _require.emitInit,
//     emitBefore = _require.emitBefore,
//     emitAfter = _require.emitAfter,
//     emitDestroy = _require.emitDestroy; // Symbols for storing async id state.


// var async_id_symbol = _Symbol('asyncId');

// var trigger_async_id_symbol = _Symbol('triggerId');

// var kHasPrimitive = _Symbol('kHasPrimitive');

// var ERR_OUT_OF_RANGE = require('internal/errors').codes.ERR_OUT_OF_RANGE;

// var _require2 = require('internal/validators'),
//     validateCallback = _require2.validateCallback,
//     validateNumber = _require2.validateNumber;

// var L = require('internal/linkedlist');

// var PriorityQueue = require('internal/priority_queue');

var _require3 = require('internal/util/inspect'),
    inspect = _require3.inspect;

var debug = require('internal/util/debuglog').debuglog('timer', function (fn) {
  debug = fn;
}); // *Must* match Environment::ImmediateInfo::Fields in src/env.h.


// var kCount = 0;
// var kRefCount = 1;
// var kHasOutstanding = 2; // Timeout values > TIMEOUT_MAX are set to 1.

// var TIMEOUT_MAX = Math.pow(2, 31) - 1;
// var timerListId = NumberMIN_SAFE_INTEGER;

// var kRefed = _Symbol('refed'); // Create a single linked list instance only once at startup


// var immediateQueue = new ImmediateList();
// var nextExpiry = Infinity;
// var refCount = 0; // This is a priority queue with a custom sorting function that first compares
// // the expiry times of two lists and if they're the same then compares their
// // individual IDs to determine which list was created first.

// var timerListQueue = new PriorityQueue(compareTimersLists, setPosition); // Object map containing linked lists of timers, keyed and sorted by their
// // duration in milliseconds.
// //
// // - key = time in milliseconds
// // - value = linked list

// var timerListMap = ObjectCreate(null);

// function initAsyncResource(resource, type) {
//   var asyncId = resource[async_id_symbol] = newAsyncId();
//   var triggerAsyncId = resource[trigger_async_id_symbol] = getDefaultTriggerAsyncId();
//   if (initHooksExist()) emitInit(asyncId, type, triggerAsyncId, resource);
// } // Timer constructor function.
// // The entire prototype is defined in lib/timers.js


// function Timeout(callback, after, args, isRepeat, isRefed) {
//   after *= 1; // Coalesce to number or NaN

//   if (!(after >= 1 && after <= TIMEOUT_MAX)) {
//     if (after > TIMEOUT_MAX) {
//       process.emitWarning("".concat(after, " does not fit into") + ' a 32-bit signed integer.' + '\nTimeout duration was set to 1.', 'TimeoutOverflowWarning');
//     }

//     after = 1; // Schedule on next tick, follows browser behavior
//   }

//   this._idleTimeout = after;
//   this._idlePrev = this;
//   this._idleNext = this;
//   this._idleStart = null; // This must be set to null first to avoid function tracking
//   // on the hidden class, revisit in V8 versions after 6.2

//   this._onTimeout = null;
//   this._onTimeout = callback;
//   this._timerArgs = args;
//   this._repeat = isRepeat ? after : null;
//   this._destroyed = false;
//   if (isRefed) incRefCount();
//   this[kRefed] = isRefed;
//   this[kHasPrimitive] = false;
//   initAsyncResource(this, 'Timeout');
// } // Make sure the linked list only shows the minimal necessary information.


// Timeout.prototype[inspect.custom] = function (_, options) {
//   return inspect(this, _objectSpread(_objectSpread({}, options), {}, {
//     // Only inspect one level.
//     depth: 0,
//     // It should not recurse.
//     customInspect: false
//   }));
// };

// Timeout.prototype.refresh = function () {
//   if (this[kRefed]) active(this);else unrefActive(this);
//   return this;
// };

// Timeout.prototype.unref = function () {
//   if (this[kRefed]) {
//     this[kRefed] = false;
//     if (!this._destroyed) decRefCount();
//   }

//   return this;
// };

// Timeout.prototype.ref = function () {
//   if (!this[kRefed]) {
//     this[kRefed] = true;
//     if (!this._destroyed) incRefCount();
//   }

//   return this;
// };

// Timeout.prototype.hasRef = function () {
//   return this[kRefed];
// };

// function TimersList(expiry, msecs) {
//   this._idleNext = this; // Create the list with the linkedlist properties to

//   this._idlePrev = this; // Prevent any unnecessary hidden class changes.

//   this.expiry = expiry;
//   this.id = timerListId++;
//   this.msecs = msecs;
//   this.priorityQueuePosition = null;
// } // Make sure the linked list only shows the minimal necessary information.


// TimersList.prototype[inspect.custom] = function (_, options) {
//   return inspect(this, _objectSpread(_objectSpread({}, options), {}, {
//     // Only inspect one level.
//     depth: 0,
//     // It should not recurse.
//     customInspect: false
//   }));
// }; // A linked list for storing `setImmediate()` requests


// function ImmediateList() {
//   this.head = null;
//   this.tail = null;
// } // Appends an item to the end of the linked list, adjusting the current tail's
// // next pointer and the item's previous pointer where applicable


// ImmediateList.prototype.append = function (item) {
//   if (this.tail !== null) {
//     this.tail._idleNext = item;
//     item._idlePrev = this.tail;
//   } else {
//     this.head = item;
//   }

//   this.tail = item;
// }; // Removes an item from the linked list, adjusting the pointers of adjacent
// // items and the linked list's head or tail pointers as necessary


// ImmediateList.prototype.remove = function (item) {
//   if (item._idleNext) {
//     item._idleNext._idlePrev = item._idlePrev;
//   }

//   if (item._idlePrev) {
//     item._idlePrev._idleNext = item._idleNext;
//   }

//   if (item === this.head) this.head = item._idleNext;
//   if (item === this.tail) this.tail = item._idlePrev;
//   item._idleNext = null;
//   item._idlePrev = null;
// };

// function incRefCount() {
//   if (refCount++ === 0) toggleTimerRef(true);
// }

// function decRefCount() {
//   if (--refCount === 0) toggleTimerRef(false);
// } // Schedule or re-schedule a timer.
// // The item must have been enroll()'d first.


// function active(item) {
//   insertGuarded(item, true);
// } // Internal APIs that need timeouts should use `unrefActive()` instead of
// // `active()` so that they do not unnecessarily keep the process open.


// function unrefActive(item) {
//   insertGuarded(item, false);
// } // The underlying logic for scheduling or re-scheduling a timer.
// //
// // Appends a timer onto the end of an existing timers list, or creates a new
// // list if one does not already exist for the specified timeout duration.


// function insertGuarded(item, refed, start) {
//   var msecs = item._idleTimeout;
//   if (msecs < 0 || msecs === undefined) return;
//   insert(item, msecs, start);
//   var isDestroyed = item._destroyed;

//   if (isDestroyed || !item[async_id_symbol]) {
//     item._destroyed = false;
//     initAsyncResource(item, 'Timeout');
//   }

//   if (isDestroyed) {
//     if (refed) incRefCount();
//   } else if (refed === !item[kRefed]) {
//     if (refed) incRefCount();else decRefCount();
//   }

//   item[kRefed] = refed;
// }

// function insert(item, msecs) {
//   var start = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : getLibuvNow();
//   // Truncate so that accuracy of sub-millisecond timers is not assumed.
//   msecs = MathTrunc(msecs);
//   item._idleStart = start; // Use an existing list if there is one, otherwise we need to make a new one.

//   var list = timerListMap[msecs];

//   if (list === undefined) {
//     debug('no %d list was found in insert, creating a new one', msecs);
//     var expiry = start + msecs;
//     timerListMap[msecs] = list = new TimersList(expiry, msecs);
//     timerListQueue.insert(list);

//     if (nextExpiry > expiry) {
//       scheduleTimer(msecs);
//       nextExpiry = expiry;
//     }
//   }

//   L.append(list, item);
// }

// function setUnrefTimeout(callback, after) {
//   // Type checking identical to setTimeout()
//   validateCallback(callback);
//   var timer = new Timeout(callback, after, undefined, false, false);
//   insert(timer, timer._idleTimeout);
//   return timer;
// } // Type checking used by timers.enroll() and Socket#setTimeout()


// function getTimerDuration(msecs, name) {
//   validateNumber(msecs, name);

//   if (msecs < 0 || !NumberIsFinite(msecs)) {
//     throw new ERR_OUT_OF_RANGE(name, 'a non-negative finite number', msecs);
//   } // Ensure that msecs fits into signed int32


//   if (msecs > TIMEOUT_MAX) {
//     process.emitWarning("".concat(msecs, " does not fit into a 32-bit signed integer.") + "\nTimer duration was truncated to ".concat(TIMEOUT_MAX, "."), 'TimeoutOverflowWarning');
//     return TIMEOUT_MAX;
//   }

//   return msecs;
// }

// function compareTimersLists(a, b) {
//   var expiryDiff = a.expiry - b.expiry;

//   if (expiryDiff === 0) {
//     if (a.id < b.id) return -1;
//     if (a.id > b.id) return 1;
//   }

//   return expiryDiff;
// }

// function setPosition(node, pos) {
//   node.priorityQueuePosition = pos;
// }

// function getTimerCallbacks(runNextTicks) {
//   // If an uncaught exception was thrown during execution of immediateQueue,
//   // this queue will store all remaining Immediates that need to run upon
//   // resolution of all error handling (if process is still alive).
//   var outstandingQueue = new ImmediateList();

//   function processImmediate() {
//     var queue = outstandingQueue.head !== null ? outstandingQueue : immediateQueue;
//     var immediate = queue.head; // Clear the linked list early in case new `setImmediate()`
//     // calls occur while immediate callbacks are executed

//     if (queue !== outstandingQueue) {
//       queue.head = queue.tail = null;
//       immediateInfo[kHasOutstanding] = 1;
//     }

//     var prevImmediate;
//     var ranAtLeastOneImmediate = false;

//     while (immediate !== null) {
//       if (ranAtLeastOneImmediate) runNextTicks();else ranAtLeastOneImmediate = true; // It's possible for this current Immediate to be cleared while executing
//       // the next tick queue above, which means we need to use the previous
//       // Immediate's _idleNext which is guaranteed to not have been cleared.

//       if (immediate._destroyed) {
//         outstandingQueue.head = immediate = prevImmediate._idleNext;
//         continue;
//       }

//       immediate._destroyed = true;
//       immediateInfo[kCount]--;
//       if (immediate[kRefed]) immediateInfo[kRefCount]--;
//       immediate[kRefed] = null;
//       prevImmediate = immediate;
//       var asyncId = immediate[async_id_symbol];
//       emitBefore(asyncId, immediate[trigger_async_id_symbol], immediate);

//       try {
//         var _immediate;

//         var argv = immediate._argv;
//         if (!argv) immediate._onImmediate();else (_immediate = immediate)._onImmediate.apply(_immediate, _toConsumableArray(argv));
//       } finally {
//         immediate._onImmediate = null;
//         if (destroyHooksExist()) emitDestroy(asyncId);
//         outstandingQueue.head = immediate = immediate._idleNext;
//       }

//       emitAfter(asyncId);
//     }

//     if (queue === outstandingQueue) outstandingQueue.head = null;
//     immediateInfo[kHasOutstanding] = 0;
//   }

//   function processTimers(now) {
//     debug('process timer lists %d', now);
//     nextExpiry = Infinity;
//     var list;
//     var ranAtLeastOneList = false;

//     while (list = timerListQueue.peek()) {
//       if (list.expiry > now) {
//         nextExpiry = list.expiry;
//         return refCount > 0 ? nextExpiry : -nextExpiry;
//       }

//       if (ranAtLeastOneList) runNextTicks();else ranAtLeastOneList = true;
//       listOnTimeout(list, now);
//     }

//     return 0;
//   }

//   function listOnTimeout(list, now) {
//     var msecs = list.msecs;
//     debug('timeout callback %d', msecs);
//     var ranAtLeastOneTimer = false;
//     var timer;

//     while (timer = L.peek(list)) {
//       var diff = now - timer._idleStart; // Check if this loop iteration is too early for the next timer.
//       // This happens if there are more timers scheduled for later in the list.

//       if (diff < msecs) {
//         list.expiry = MathMax(timer._idleStart + msecs, now + 1);
//         list.id = timerListId++;
//         timerListQueue.percolateDown(1);
//         debug('%d list wait because diff is %d', msecs, diff);
//         return;
//       }

//       if (ranAtLeastOneTimer) runNextTicks();else ranAtLeastOneTimer = true; // The actual logic for when a timeout happens.

//       L.remove(timer);
//       var asyncId = timer[async_id_symbol];

//       if (!timer._onTimeout) {
//         if (!timer._destroyed) {
//           timer._destroyed = true;
//           if (timer[kRefed]) refCount--;
//           if (destroyHooksExist()) emitDestroy(asyncId);
//         }

//         continue;
//       }

//       emitBefore(asyncId, timer[trigger_async_id_symbol], timer);
//       var start = void 0;
//       if (timer._repeat) start = getLibuvNow();

//       try {
//         var args = timer._timerArgs;
//         if (args === undefined) timer._onTimeout();else ReflectApply(timer._onTimeout, timer, args);
//       } finally {
//         if (timer._repeat && timer._idleTimeout !== -1) {
//           timer._idleTimeout = timer._repeat;
//           insert(timer, timer._idleTimeout, start);
//         } else if (!timer._idleNext && !timer._idlePrev && !timer._destroyed) {
//           timer._destroyed = true;
//           if (timer[kRefed]) refCount--;
//           if (destroyHooksExist()) emitDestroy(asyncId);
//         }
//       }

//       emitAfter(asyncId);
//     } // If `L.peek(list)` returned nothing, the list was either empty or we have
//     // called all of the timer timeouts.
//     // As such, we can remove the list from the object map and
//     // the PriorityQueue.


//     debug('%d list empty', msecs); // The current list may have been removed and recreated since the reference
//     // to `list` was created. Make sure they're the same instance of the list
//     // before destroying.

//     if (list === timerListMap[msecs]) {
//       delete timerListMap[msecs];
//       timerListQueue.shift();
//     }
//   }

//   return {
//     processImmediate: processImmediate,
//     processTimers: processTimers
//   };
// }

// var Immediate = /*#__PURE__*/function () {
//   function Immediate(callback, args) {
//     _classCallCheck(this, Immediate);

//     this._idleNext = null;
//     this._idlePrev = null;
//     this._onImmediate = callback;
//     this._argv = args;
//     this._destroyed = false;
//     this[kRefed] = false;
//     initAsyncResource(this, 'Immediate');
//     this.ref();
//     immediateInfo[kCount]++;
//     immediateQueue.append(this);
//   }

//   _createClass(Immediate, [{
//     key: "ref",
//     value: function ref() {
//       if (this[kRefed] === false) {
//         this[kRefed] = true;
//         if (immediateInfo[kRefCount]++ === 0) toggleImmediateRef(true);
//       }

//       return this;
//     }
//   }, {
//     key: "unref",
//     value: function unref() {
//       if (this[kRefed] === true) {
//         this[kRefed] = false;
//         if (--immediateInfo[kRefCount] === 0) toggleImmediateRef(false);
//       }

//       return this;
//     }
//   }, {
//     key: "hasRef",
//     value: function hasRef() {
//       return !!this[kRefed];
//     }
//   }]);

//   return Immediate;
// }();

module.exports = {
//   TIMEOUT_MAX: TIMEOUT_MAX,
  kTimeout: _Symbol('timeout'),
  // For hiding Timeouts on other internals.
//   async_id_symbol: async_id_symbol,
//   trigger_async_id_symbol: trigger_async_id_symbol,
//   Timeout: Timeout,
//   Immediate: Immediate,
//   kRefed: kRefed,
//   kHasPrimitive: kHasPrimitive,
//   initAsyncResource: initAsyncResource,
//   setUnrefTimeout: setUnrefTimeout,
//   getTimerDuration: getTimerDuration,
//   immediateQueue: immediateQueue,
//   getTimerCallbacks: getTimerCallbacks,
//   immediateInfoFields: {
//     kCount: kCount,
//     kRefCount: kRefCount,
//     kHasOutstanding: kHasOutstanding
//   },
//   active: active,
//   unrefActive: unrefActive,
//   insert: insert,
//   timerListMap: timerListMap,
//   timerListQueue: timerListQueue,
//   decRefCount: decRefCount,
//   incRefCount: incRefCount
};
