/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: true

const copy1 = require('./cjs-caching-2.js');
copy1.recreateExports();
const copy2 = require('./cjs-caching-2.js');
print('With a dependency cycle:');
print(copy1 === copy2 ? 'Copies are equal' : 'Copies are different');
