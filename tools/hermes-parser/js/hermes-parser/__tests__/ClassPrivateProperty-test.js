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

import type {AlignmentCase} from '../__test_utils__/alignment-utils';

import {
  expectBabelAlignment,
  expectEspreeAlignment,
} from '../__test_utils__/alignment-utils';
import {parse, parseForSnapshot} from '../__test_utils__/parse';

const PRIVATE_ERROR_MESSAGE = 'Private properties are not supported';
describe('Private properties', () => {
  describe('Property Definition', () => {
    const testCase: AlignmentCase = {
      code: `
        class Foo {
          #private;
        }
      `,
      espree: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
      babel: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
    };

    test('ESTree', () => {
      // Private property uses are not supported
      expect(() => parseForSnapshot(testCase.code)).toThrow(
        new SyntaxError(`${PRIVATE_ERROR_MESSAGE} (3:10)`),
      );
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Private property uses are not supported
      expect(() => parse(testCase.code, {babel: true})).toThrow(
        new SyntaxError('Private properties are not supported (3:10)'),
      );
      expectBabelAlignment(testCase);
    });
  });

  describe('Member Expression', () => {
    const testCase: AlignmentCase = {
      code: `
        class Foo {
          #private;
          constructor() {
            foo.#private;
          }
        }
      `,
      espree: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
      babel: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
    };

    test('ESTree', () => {
      // Private property uses are not supported
      expect(() => parseForSnapshot(testCase.code)).toThrow(
        new SyntaxError(`${PRIVATE_ERROR_MESSAGE} (3:10)`),
      );
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Private property uses are not supported
      expect(() => parse(testCase.code, {babel: true})).toThrow(
        new SyntaxError('Private properties are not supported (3:10)'),
      );
      expectBabelAlignment(testCase);
    });
  });

  describe('Brand Check', () => {
    const testCase: AlignmentCase = {
      code: `
        class Foo {
          #private;
          constructor() {
            #private in foo;
          }
        }
      `,
      espree: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
      babel: {
        expectToFail: 'hermes-exception',
        expectedExceptionMessage: PRIVATE_ERROR_MESSAGE,
      },
    };

    test('ESTree', () => {
      // Private property uses are not supported
      expect(() => parseForSnapshot(testCase.code)).toThrow(
        new SyntaxError(`${PRIVATE_ERROR_MESSAGE} (3:10)`),
      );
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      // Private property uses are not supported
      expect(() => parse(testCase.code, {babel: true})).toThrow(
        new SyntaxError('Private properties are not supported (3:10)'),
      );
      expectBabelAlignment(testCase);
    });
  });
});
