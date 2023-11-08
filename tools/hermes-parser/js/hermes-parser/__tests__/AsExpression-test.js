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

import {
  parseForSnapshotBabel,
  parseForSnapshotESTree,
  printForSnapshotBabel,
} from '../__test_utils__/parse';

describe('`as` expression', () => {
  const code = 'x as number;';

  test('ESTree', async () => {
    expect(await parseForSnapshotESTree(code)).toMatchSnapshot();
  });

  test('Babel', async () => {
    expect(await parseForSnapshotBabel(code)).toMatchSnapshot();
    expect(await printForSnapshotBabel(code)).toMatchInlineSnapshot(
      `"(x: number);"`,
    );
  });
});
