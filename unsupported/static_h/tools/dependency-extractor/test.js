/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const hc = require('./DependencyExtractor.js');
const assert = require('assert');

function getDepsOrError(str) {
  try {
    let deps = hc.extractDependencies(str);
    return deps;
  } catch (e) {
    return e;
  }
}

function wrap(func) {
  try {
    return func();
  } catch (e) {
    return e.message;
  }
}

let deps;

deps = getDepsOrError(`
  require('foo');
  import * as Bar from 'bar';
  `);
assert.deepEqual(deps, [
  {
    name: 'foo',
    kind: 'Require',
  },
  {
    name: 'bar',
    kind: 'ESM',
  },
]);

deps = getDepsOrError("require('foo');");
assert.deepEqual(deps, [
  {
    name: 'foo',
    kind: 'Require',
  },
]);

console.log("Tests passed!");
