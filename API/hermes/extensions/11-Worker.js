/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Worker extension setup function.
// Receives native helper functions and installs Worker globally.
extensions.Worker = function(nativeInit, nativeTerminate, nativePostMessage) {
    class Worker {
        constructor(script) {
            nativeInit(this, script);
        }
        terminate() {
            return nativeTerminate.call(this);
        }
        postMessage(...args) {
            return nativePostMessage.call(this, ...args);
        }
    }

    globalThis.Worker = Worker;
};
