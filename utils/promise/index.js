// @nolint
// This file is used to generate InternalBytecode/Promise.js
// See InternalBytecode/README.md for more details.

const Promise = require('promise/setimmediate/es6-extensions');

require('promise/setimmediate/finally');

// expose Promise to global.
globalThis.Promise = Promise;

// register the JavaScript implemented `enable` function into
// the Hermes' internal promise rejection tracker.
var enableHook = require('promise/setimmediate/rejection-tracking').enable
HermesInternal?.setPromiseRejectionTrackingHook?.(enableHook);
