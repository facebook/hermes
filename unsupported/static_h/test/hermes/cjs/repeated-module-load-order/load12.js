/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: true

loadSegment(require.context, 1);
require('./seg1.js');
require('./shared12.js')();
loadSegment(require.context, 2);
require('./seg2.js');
require('./shared12.js')();
