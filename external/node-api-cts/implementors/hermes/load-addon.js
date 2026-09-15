/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Provides the loadAddon global for CTS tests running under Hermes.
// Expects __napiAddonDir to be set before this file is evaluated,
// containing the absolute path to the directory where .node files
// are located.
function loadAddon(addonName) {
  if (typeof addonName !== 'string') {
    throw new TypeError('loadAddon: expected a string argument');
  }
  var path = __napiAddonDir + '/' + addonName + '.node';
  return loadNativeModule(path);
}
