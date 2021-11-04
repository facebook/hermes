/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {transform} from '../../src/transform/transform';

describe('transform', () => {
  it('should do nothing (including no formatting) if no mutations are applied', () => {
    const code = 'const x = 1'; // no semi to ensure no formatting
    const result = transform(code, () => ({}));

    expect(result).toBe(code);
  });

  // TODO - turn this on when we have the replaceNode mutation
  it.skip('should format the code as well as mutate it', () => {
    const code = 'const x = 1'; // no semi to ensure no formatting
    const result = transform(code, context => ({
      Identifier(node) {
        if (node.type !== 'Identifier') {
          return;
        }

        context.replaceNode(
          node,
          context.shallowCloneNode(node, {
            name: 'y',
          }),
        );
      },
    }));

    expect(result).toBe('const y = 1;');
  });
});
