/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {ESNode} from 'hermes-estree';

import {VisitorKeys} from 'hermes-eslint';

export function getVisitorKeys<T: ESNode>(node: T): $ReadOnlyArray<$Keys<T>> {
  const keys = VisitorKeys[node.type];
  if (keys == null) {
    throw new Error(`No visitor keys found for node type "${node.type}".`);
  }

  // $FlowExpectedError[prop-missing]
  return keys;
}
