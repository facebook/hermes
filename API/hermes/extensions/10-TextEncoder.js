/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// TextEncoder extension setup function.
// Receives native helper functions and installs TextEncoder globally.
  extensions.TextEncoder = function(nativeInit, nativeEncode, nativeEncodeInto) {
    function TextEncoder() {
      if (!new.target) {
        throw new TypeError('TextEncoder must be called as a constructor');
      }
      nativeInit(this);
    }

    Object.defineProperty(TextEncoder.prototype, 'encoding', {
      get: function() {
        return 'utf-8';
      },
      enumerable: true,
      configurable: true
    });

    TextEncoder.prototype.encode = nativeEncode;
    TextEncoder.prototype.encodeInto = nativeEncodeInto;

    Object.defineProperty(TextEncoder.prototype, Symbol.toStringTag, {
      value: 'TextEncoder',
      writable: false,
      enumerable: false,
      configurable: true
    });

    globalThis.TextEncoder = TextEncoder;
  };
