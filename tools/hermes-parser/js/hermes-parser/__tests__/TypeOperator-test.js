/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {
  printForSnapshotESTree,
  parseForSnapshotESTree,
  printForSnapshotBabel,
  parseForSnapshotBabel,
} from '../__test_utils__/parse';

describe('TypeOperator', () => {
  describe('renders', () => {
    describe('Basic', () => {
      const code = `
        type T = renders Foo;
      `;

      test('ESTree', async () => {
        expect(await parseForSnapshotESTree(code)).toMatchSnapshot();
        expect(await printForSnapshotESTree(code)).toBe(code.trim());
      });

      test('Babel', async () => {
        expect(await parseForSnapshotBabel(code)).toMatchSnapshot();
        expect(await printForSnapshotBabel(code)).toMatchInlineSnapshot(
          `"type T = any;"`,
        );
      });
    });
    describe('Union', () => {
      const code = `
        type T = renders (Foo | Bar);
      `;

      test('ESTree', async () => {
        expect(await parseForSnapshotESTree(code)).toMatchSnapshot();
        expect(await printForSnapshotESTree(code)).toBe(code.trim());
      });

      test('Babel', async () => {
        expect(await parseForSnapshotBabel(code)).toMatchSnapshot();
        expect(await printForSnapshotBabel(code)).toMatchInlineSnapshot(
          `"type T = any;"`,
        );
      });
    });
    describe('Nested Union', () => {
      const code = `
        type T = renders (Foo | Bar) | null;
      `;

      test('ESTree', async () => {
        expect(await parseForSnapshotESTree(code)).toMatchSnapshot();
        expect(await printForSnapshotESTree(code)).toBe(code.trim());
      });

      test('Babel', async () => {
        expect(await parseForSnapshotBabel(code)).toMatchSnapshot();
        expect(await printForSnapshotBabel(code)).toMatchInlineSnapshot(
          `"type T = any | null;"`,
        );
      });
    });
  });
});
