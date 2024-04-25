/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {AlignmentCase} from '../__test_utils__/alignment-utils';

import {expectEspreeAlignment} from '../__test_utils__/alignment-utils';
import {parseForSnapshot} from '../__test_utils__/parse';

describe('Enum', () => {
  const testCase: AlignmentCase = {
    code: `
      enum T1 { A = 1n, B = 2n}
    `,
    espree: {
      expectToFail: 'espree-exception',
      expectedExceptionMessage: `The keyword 'enum' is reserved`,
    },
    babel: {
      expectToFail: 'babel-exception',
      expectedExceptionMessage: 'Unexpected token',
    },
  };

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      {
        "body": [
          {
            "body": {
              "explicitType": false,
              "hasUnknownMembers": false,
              "members": [
                {
                  "id": {
                    "name": "A",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "init": {
                    "bigint": "1",
                    "literalType": "bigint",
                    "raw": "1n",
                    "type": "Literal",
                    "value": 1n,
                  },
                  "type": "EnumBigIntMember",
                },
                {
                  "id": {
                    "name": "B",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "init": {
                    "bigint": "2",
                    "literalType": "bigint",
                    "raw": "2n",
                    "type": "Literal",
                    "value": 2n,
                  },
                  "type": "EnumBigIntMember",
                },
              ],
              "type": "EnumBigIntBody",
            },
            "id": {
              "name": "T1",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "type": "EnumDeclaration",
          },
        ],
        "type": "Program",
      }
    `);
    expectEspreeAlignment(testCase);
  });
});
