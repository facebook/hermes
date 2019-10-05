// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.

(function() {
  'use strict';

  var desc = {
    enumerable: false,
    writable: false,
    configurable: false,
    value: 9007199254740991,
  };

  // ES6.0 20.1.2.6
  Object.defineProperty(Number, 'MAX_SAFE_INTEGER', desc);

  desc.value = -desc.value;

  // ES6.0 20.1.2.8
  Object.defineProperty(Number, 'MIN_SAFE_INTEGER', desc);
})();
