/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: true

loadSegment(require.context, 4);
require('./seg4.js');
require('./shared34.js')();
loadSegment(require.context, 3);
require('./seg3.js');
require('./shared34.js')();
