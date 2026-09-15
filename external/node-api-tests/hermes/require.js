/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Provides a require() shim for running Node.js NAPI tests under Hermes.
// Expects __testDir to be set by the test runner to the test directory path.
// Expects __addonDir to be set by the test runner to the addon directory path.
// Expects __assert and __common to be defined by assert.js and common.js.

(function() {
'use strict';

// Cache for loaded modules.
var moduleCache = {};

globalThis.require = function require(specifier) {
  if (moduleCache[specifier] !== undefined) {
    return moduleCache[specifier];
  }

  // Built-in module: assert
  if (specifier === 'assert') {
    moduleCache[specifier] = __assert;
    return __assert;
  }

  // Built-in module: common (Node.js test helpers)
  if (specifier === '../../common' || specifier === '../common') {
    moduleCache[specifier] = __common;
    return __common;
  }

  // Common GC helper
  if (specifier === '../../common/gc' || specifier === '../common/gc') {
    moduleCache[specifier] = __gc;
    return __gc;
  }

  // Native addon: ./build/Release/addon_name or ./build/Debug/addon_name
  var addonMatch = specifier.match(
    /^\.\/build\/(?:Release|Debug)\/(.+)$/
  );
  if (addonMatch) {
    var addonName = addonMatch[1];
    var addonPath = __addonDir + '/' + addonName + '.node';
    var addon = loadNativeModule(addonPath);
    moduleCache[specifier] = addon;
    return addon;
  }

  // Unsupported module - throw an error like Node.js would.
  throw new Error(
    'Cannot find module \'' + specifier + '\' ' +
    '(not supported in Hermes test harness)'
  );
};

// Support for template literal require paths.
// Node tests use: require(`./build/${common.buildType}/name`)
// After template evaluation this becomes: require('./build/Release/name')
// which is handled above. Addons are loaded from __addonDir (build tree).

})();
