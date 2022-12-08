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

import {expectBabelAlignment} from '../__test_utils__/alignment-utils';
import {parse, parseForSnapshot} from '../__test_utils__/parse';

describe('Literal', () => {
  const testCase: AlignmentCase = {
    code: `
      type T1 = 10;
      type T2 = 0.56283;
      type T3 = "test";
      type T4 = true;
      type T5 = 4321n;
      type T6 = 12_34n;
    `,
    espree: {expectToFail: false},
    babel: {expectToFail: false},
  };

  test('Emitted `.value` type is correct', () => {
    // Also assert that the literal's `.value` is the correct instance type
    expect(parse(testCase.code)).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'TypeAlias',
          right: {
            type: 'NumberLiteralTypeAnnotation',
            value: 10,
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'NumberLiteralTypeAnnotation',
            value: 0.56283,
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'StringLiteralTypeAnnotation',
            value: 'test',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'BooleanLiteralTypeAnnotation',
            value: true,
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'BigIntLiteralTypeAnnotation',
            // $FlowExpectedError[cannot-resolve-name] - not supported by flow yet
            value: BigInt(4321),
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'BigIntLiteralTypeAnnotation',
            // $FlowExpectedError[cannot-resolve-name] - not supported by flow yet
            value: BigInt(1234),
          },
        },
      ],
    });
  });

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      {
        "body": [
          {
            "id": {
              "name": "T1",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "raw": "10",
              "type": "NumberLiteralTypeAnnotation",
              "value": 10,
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T2",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "raw": "0.56283",
              "type": "NumberLiteralTypeAnnotation",
              "value": 0.56283,
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T3",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "raw": ""test"",
              "type": "StringLiteralTypeAnnotation",
              "value": "test",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T4",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "raw": "true",
              "type": "BooleanLiteralTypeAnnotation",
              "value": true,
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T5",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "bigint": "4321",
              "raw": "4321n",
              "type": "BigIntLiteralTypeAnnotation",
              "value": 4321n,
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T6",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "bigint": "1234",
              "raw": "12_34n",
              "type": "BigIntLiteralTypeAnnotation",
              "value": 1234n,
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
        ],
        "type": "Program",
      }
    `);
    // types-only so no espree alignment possible
  });

  test('Babel', () => {
    // Babel AST literal nodes
    expect(parse(testCase.code, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'TypeAlias',
            right: {
              type: 'NumberLiteralTypeAnnotation',
              value: 10,
            },
          },
          {
            type: 'TypeAlias',
            right: {
              type: 'NumberLiteralTypeAnnotation',
              value: 0.56283,
            },
          },
          {
            type: 'TypeAlias',
            right: {
              type: 'StringLiteralTypeAnnotation',
              value: 'test',
            },
          },
          {
            type: 'TypeAlias',
            right: {
              type: 'BooleanLiteralTypeAnnotation',
              value: true,
            },
          },
          {
            type: 'TypeAlias',
            right: {
              type: 'BigIntLiteralTypeAnnotation',
              // $FlowExpectedError[cannot-resolve-name] - not supported by flow yet
              value: BigInt(4321),
            },
          },
          {
            type: 'TypeAlias',
            right: {
              type: 'BigIntLiteralTypeAnnotation',
              // $FlowExpectedError[cannot-resolve-name] - not supported by flow yet
              value: BigInt(1234),
            },
          },
        ],
      },
    });
    expectBabelAlignment(testCase);
  });
});

describe('Keyword Types', () => {
  const testCase: AlignmentCase = {
    code: `
      type T1 = boolean;
      type T2 = string;
      type T3 = number;
      type T4 = bigint;
      type T5 = any;
      type T6 = empty;
      type T7 = symbol;
      type T8 = mixed;
      type T9 = void;
      type T0 = null;
    `,
    espree: {expectToFail: false},
    babel: {expectToFail: false},
  };

  test('Emitted `.value` type is correct', () => {
    // Also assert that the literal's `.value` is the correct instance type
    expect(parse(testCase.code)).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'TypeAlias',
          right: {
            type: 'BooleanTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'StringTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'NumberTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'BigIntTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'AnyTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'EmptyTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'SymbolTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'MixedTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'VoidTypeAnnotation',
          },
        },
        {
          type: 'TypeAlias',
          right: {
            type: 'NullLiteralTypeAnnotation',
          },
        },
      ],
    });
  });

  it('estree AST', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      {
        "body": [
          {
            "id": {
              "name": "T1",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "BooleanTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T2",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "StringTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T3",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "NumberTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T4",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "BigIntTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T5",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "AnyTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T6",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "EmptyTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T7",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "SymbolTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T8",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "MixedTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T9",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "VoidTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
          {
            "id": {
              "name": "T0",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": {
              "type": "NullLiteralTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
        ],
        "type": "Program",
      }
    `);
  });
  it(`should match babel`, () => {
    expectBabelAlignment(testCase);
  });
});
