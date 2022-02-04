/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

require('cjs-subdir-shared.min.js').immediatelyInvoke(
  function mainInner() {
    loadSegment(require.context, 2);
    var unmin = require('cjs-subdir-unminified.js');
    unmin.run();
  }
);
