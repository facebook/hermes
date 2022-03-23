/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

export default function keyMirror<T: {...}>(obj: T): $KeyMirror<T> {
  const ret: {[string]: string} = {};
  for (const key of Object.keys(obj)) {
    ret[key] = key;
  }
  return ret;
}
