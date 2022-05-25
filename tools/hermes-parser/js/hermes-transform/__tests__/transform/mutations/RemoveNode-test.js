/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {
  PropertyDefinition,
  EnumDefaultedMember,
  FunctionTypeParam,
  Identifier,
  ObjectTypeIndexer,
} from 'hermes-estree';

import {
  createRemoveNodeMutation,
  performRemoveNodeMutation,
} from '../../../src/transform/mutations/RemoveNode';
import {MutationContext} from '../../../src/transform/MutationContext';
import {parseAndGetAstAndNode} from './test-utils';

describe('RemoveNode', () => {
  it('PropertyDefinition', () => {
    const {ast, target} = parseAndGetAstAndNode<PropertyDefinition>(
      'PropertyDefinition',
      'class Foo { prop = 1; method() {} }',
    );
    const mutation = createRemoveNodeMutation(target);
    performRemoveNodeMutation(new MutationContext(''), mutation);
    expect(ast).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'ClassDeclaration',
          body: {
            type: 'ClassBody',
            body: [
              {
                type: 'MethodDefinition',
              },
            ],
          },
        },
      ],
    });
  });

  it('EnumDefaultedMember', () => {
    const {ast, target} = parseAndGetAstAndNode<EnumDefaultedMember>(
      'EnumDefaultedMember',
      'enum Foo { A, B }',
    );
    const mutation = createRemoveNodeMutation(target);
    performRemoveNodeMutation(new MutationContext(''), mutation);
    expect(ast).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'EnumDeclaration',
          body: {
            type: 'EnumStringBody',
            members: [
              {
                type: 'EnumDefaultedMember',
                id: {
                  type: 'Identifier',
                  name: 'A',
                },
              },
            ],
          },
        },
      ],
    });
  });

  it('FunctionTypeParam', () => {
    const {ast, target} = parseAndGetAstAndNode<FunctionTypeParam>(
      'FunctionTypeParam',
      'type T = (string, number) => void',
    );
    const mutation = createRemoveNodeMutation(target);
    performRemoveNodeMutation(new MutationContext(''), mutation);
    expect(ast).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'TypeAlias',
          right: {
            type: 'FunctionTypeAnnotation',
            params: [
              {
                type: 'FunctionTypeParam',
                typeAnnotation: {
                  type: 'StringTypeAnnotation',
                },
              },
            ],
          },
        },
      ],
    });
  });

  it('ObjectTypeIndexer', () => {
    const {ast, target} = parseAndGetAstAndNode<ObjectTypeIndexer>(
      'ObjectTypeIndexer',
      'type T = {prop: string, [number]: string, [string]: number};',
    );
    const mutation = createRemoveNodeMutation(target);
    performRemoveNodeMutation(new MutationContext(''), mutation);
    expect(ast).toMatchObject({
      type: 'Program',
      body: [
        {
          type: 'TypeAlias',
          right: {
            type: 'ObjectTypeAnnotation',
            properties: [
              {
                type: 'ObjectTypeProperty',
              },
            ],
            indexers: [
              {
                type: 'ObjectTypeIndexer',
                key: {
                  type: 'NumberTypeAnnotation',
                },
                value: {
                  type: 'StringTypeAnnotation',
                },
              },
            ],
          },
        },
      ],
    });
  });

  describe('Identifier', () => {
    it('valid', () => {
      const {ast, target} = parseAndGetAstAndNode<Identifier>(
        'Identifier',
        '[1, a, 3];',
      );
      const mutation = createRemoveNodeMutation(target);
      performRemoveNodeMutation(new MutationContext(''), mutation);
      expect(ast).toMatchObject({
        type: 'Program',
        body: [
          {
            type: 'ExpressionStatement',
            expression: {
              type: 'ArrayExpression',
              elements: [
                {
                  type: 'Literal',
                  value: 1,
                },
                {
                  type: 'Literal',
                  value: 3,
                },
              ],
            },
          },
        ],
      });
    });

    it('invalid', () => {
      const {target} = parseAndGetAstAndNode<Identifier>(
        'Identifier',
        'const x = 1;',
      );
      const mutation = createRemoveNodeMutation(target);
      expect(() => performRemoveNodeMutation(new MutationContext(''), mutation))
        .toThrowErrorMatchingInlineSnapshot(`
        "Tried to remove Identifier from parent of type VariableDeclarator.
        However Identifier can only be safely removed from parent of type ArrowFunctionExpression | FunctionDeclaration | FunctionExpression | ArrayExpression | ArrayPattern."
      `);
    });
  });
});
