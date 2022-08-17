/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import {SimpleTraverser} from 'hermes-transform';
import {parse as parseOriginal} from '../src/index';

export const parse: typeof parseOriginal = (source, options) => {
  // $FlowExpectedError[incompatible-call] - the overloads confuse flow
  return parseOriginal(source, {flow: 'all', ...options});
};

export function parseForSnapshot(
  source: string,
  options?: {preserveRange?: boolean},
): mixed {
  const program = parse(source);
  SimpleTraverser.traverse(program, {
    enter(node) {
      // $FlowExpectedError[cannot-write]
      delete node.loc;
      // $FlowExpectedError[cannot-write]
      delete node.parent;
      if (options?.preserveRange !== true) {
        // $FlowExpectedError[cannot-write]
        delete node.range;
      }
    },
    leave() {},
  });

  return {
    type: 'Program',
    body: program.body,
  };
}
