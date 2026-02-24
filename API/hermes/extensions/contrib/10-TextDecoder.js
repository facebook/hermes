/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// TextDecoder extension setup function.
// Receives native helper functions and installs TextDecoder globally.
  extensions.TextDecoder = function(nativeInit, nativeDecode, nativeGetEncoding, nativeGetFatal, nativeGetIgnoreBOM) {
    function TextDecoder(label, options) {
      if (!new.target) {
        throw new TypeError('TextDecoder must be called as a constructor');
      }
      nativeInit(this, label, options);
    }

    Object.defineProperty(TextDecoder.prototype, 'encoding', {
      get: function() {
        return nativeGetEncoding(this);
      },
      enumerable: true,
      configurable: true
    });

    Object.defineProperty(TextDecoder.prototype, 'fatal', {
      get: function() {
        return nativeGetFatal(this);
      },
      enumerable: true,
      configurable: true
    });

    Object.defineProperty(TextDecoder.prototype, 'ignoreBOM', {
      get: function() {
        return nativeGetIgnoreBOM(this);
      },
      enumerable: true,
      configurable: true
    });

    TextDecoder.prototype.decode = nativeDecode;

    Object.defineProperty(TextDecoder.prototype, Symbol.toStringTag, {
      value: 'TextDecoder',
      writable: false,
      enumerable: false,
      configurable: true
    });

    globalThis.TextDecoder = TextDecoder;
  };
