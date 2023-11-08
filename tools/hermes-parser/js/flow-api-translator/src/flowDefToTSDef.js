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

import type {ObjectWithLoc} from 'hermes-estree';
import * as FlowESTree from 'hermes-estree';
import type {ScopeManager} from 'hermes-eslint';
import {
  cloneJSDocCommentsToNewNode as cloneJSDocCommentsToNewNodeOriginal,
  makeCommentOwnLine as makeCommentOwnLineOriginal,
} from 'hermes-transform';
import * as TSESTree from './utils/ts-estree-ast-types';
import {
  buildCodeFrame,
  translationError as translationErrorBase,
  unexpectedTranslationError as unexpectedTranslationErrorBase,
} from './utils/ErrorUtils';
import {removeAtFlowFromDocblock} from './utils/DocblockUtils';
import type {TranslationOptions} from './utils/TranslationUtils';
import {EOL} from 'os';

type DeclarationOrUnsupported<T> = T | TSESTree.TSTypeAliasDeclaration;

const cloneJSDocCommentsToNewNode =
  // $FlowExpectedError[incompatible-cast] - trust me this re-type is 100% safe
  (cloneJSDocCommentsToNewNodeOriginal: (mixed, mixed) => void);

const makeCommentOwnLine =
  // $FlowExpectedError[incompatible-cast] - trust me this re-type is 100% safe
  (makeCommentOwnLineOriginal: (string, mixed) => string);

const VALID_REACT_IMPORTS = new Set<string>(['React', 'react']);

function isValidReactImportOrGlobal(id: FlowESTree.Identifier): boolean {
  return VALID_REACT_IMPORTS.has(id.name) || id.name.startsWith('React$');
}

let shouldAddReactImport: boolean | null = null;

export function flowDefToTSDef(
  originalCode: string,
  ast: FlowESTree.Program,
  scopeManager: ScopeManager,
  opts: TranslationOptions,
): [TSESTree.Program, string] {
  const tsBody: Array<TSESTree.ProgramStatement> = [];
  const tsProgram: TSESTree.Program = {
    type: 'Program',
    body: tsBody,
    sourceType: ast.sourceType,
    docblock:
      ast.docblock == null ? null : removeAtFlowFromDocblock(ast.docblock),
  };

  shouldAddReactImport = null;

  const [transform, code] = getTransforms(originalCode, scopeManager, opts);

  for (const node of ast.body) {
    if (node.type in transform) {
      const result: $FlowFixMe | Array<$FlowFixMe> = transform[
        // $FlowExpectedError[prop-missing]
        node.type
      ](
        // $FlowExpectedError[incompatible-type]
        // $FlowExpectedError[prop-missing]
        // $FlowFixMe[incompatible-call]
        node,
      );
      tsBody.push(...(Array.isArray(result) ? result : [result]));
    } else {
      throw unexpectedTranslationErrorBase(
        node,
        `Unexpected node type ${node.type}`,
        {code},
      );
    }
  }

  if (shouldAddReactImport === true) {
    tsBody.unshift({
      type: 'ImportDeclaration',
      assertions: [],
      source: {
        type: 'Literal',
        value: 'react',
        raw: "'react'",
      },
      specifiers: [
        {
          type: 'ImportNamespaceSpecifier',
          local: {
            type: 'Identifier',
            name: 'React',
          },
        },
      ],
      importKind: 'value',
    });
  }

  return [tsProgram, code];
}

const getTransforms = (
  originalCode: string,
  scopeManager: ScopeManager,
  opts: TranslationOptions,
) => {
  let code = originalCode;
  function translationError(node: ObjectWithLoc, message: string) {
    return translationErrorBase(node, message, {code});
  }
  function unexpectedTranslationError(node: ObjectWithLoc, message: string) {
    return unexpectedTranslationErrorBase(node, message, {code});
  }
  function unsupportedFeatureMessage(thing: string) {
    return `Unsupported feature: Translating "${thing}" is currently not supported.`;
  }
  function buildCodeFrameForComment(node: ObjectWithLoc, message: string) {
    return buildCodeFrame(node, message, code, false);
  }
  function addErrorComment(node: TSESTree.Node, message: string): void {
    const comment = {
      type: 'Block',
      value: `*${EOL} * ${message.replace(
        new RegExp(EOL, 'g'),
        `${EOL} * `,
      )}${EOL}*`,
      leading: true,
      printed: false,
    };

    code = makeCommentOwnLine(code, comment);

    // $FlowExpectedError[prop-missing]
    // $FlowExpectedError[cannot-write]
    node.comments ??= [];
    // $FlowExpectedError[prop-missing]
    // $FlowExpectedError[incompatible-cast]
    (node.comments: Array<TSESTree.Comment>).push(comment);
  }
  function unsupportedAnnotation(
    node: ObjectWithLoc,
    thing: string,
  ): TSESTree.TSAnyKeyword {
    const message = unsupportedFeatureMessage(thing);
    if (opts.recoverFromErrors) {
      const codeFrame = buildCodeFrameForComment(node, message);
      const newNode = {
        type: 'TSAnyKeyword',
      };
      addErrorComment(newNode, codeFrame);
      return newNode;
    }

    throw translationError(node, message);
  }
  function unsupportedDeclaration(
    node: ObjectWithLoc,
    thing: string,
    id: FlowESTree.Identifier,
    declare: boolean = false,
    typeParameters: FlowESTree.TypeParameterDeclaration | null = null,
  ): TSESTree.TSTypeAliasDeclaration {
    const message = unsupportedFeatureMessage(thing);
    if (opts.recoverFromErrors) {
      const codeFrame = buildCodeFrameForComment(node, message);
      const newNode = {
        type: 'TSTypeAliasDeclaration',
        declare,
        id: transform.Identifier(id, false),
        typeAnnotation: {
          type: 'TSAnyKeyword',
        },
        typeParameters:
          typeParameters == null
            ? undefined
            : transform.TypeParameterDeclaration(typeParameters),
      };
      addErrorComment(newNode, codeFrame);
      return newNode;
    }

    throw translationError(node, message);
  }

  const topScope = (() => {
    const globalScope = scopeManager.globalScope;
    if (
      globalScope.childScopes.length > 0 &&
      globalScope.childScopes[0].type === 'module'
    ) {
      return globalScope.childScopes[0];
    }
    return globalScope;
  })();

  function isReactImport(id: FlowESTree.Identifier): boolean {
    let currentScope = (() => {
      let scope = null;
      let node: FlowESTree.ESNode = id;
      while (!scope && node) {
        scope = scopeManager.acquire(node, true);
        node = node.parent;
      }

      return scope;
    })();

    if (currentScope == null) {
      throw new Error('unable to resolve scope');
    }

    const variableDef = (() => {
      while (currentScope != null) {
        for (const variable of currentScope.variables) {
          if (variable.defs.length && variable.name === id.name) {
            return variable;
          }
        }
        currentScope = currentScope.upper;
      }
    })();

    // No variable found, it is not imported.
    // It could be a global though if isValidReactImportOrGlobal returns true.
    if (variableDef == null) {
      return false;
    }

    const def = variableDef.defs[0];
    // Detect:
    switch (def.type) {
      // import React from 'react';
      // import * as React from 'react';
      case 'ImportBinding': {
        if (
          def.node.type === 'ImportDefaultSpecifier' ||
          def.node.type === 'ImportNamespaceSpecifier'
        ) {
          return VALID_REACT_IMPORTS.has(def.parent.source.value);
        }
        return false;
      }

      // Globals
      case 'ImplicitGlobalVariable': {
        return VALID_REACT_IMPORTS.has(id.name);
      }

      // TODO Handle:
      // const React = require('react');
      // const Something = React;
    }

    return false;
  }

  function EnumImpl(
    node: FlowESTree.EnumDeclaration | FlowESTree.DeclareEnum,
  ): DeclarationOrUnsupported<
    [TSESTree.TSEnumDeclaration, TSESTree.TSModuleDeclaration],
  > {
    const body = node.body;
    if (body.type === 'EnumSymbolBody') {
      /*
      There's unfortunately no way for us to support this in a clean way.
      We can get really close using this code:
      ```
      declare namespace SymbolEnum {
          export const member1: unique symbol;
          export type member1 = typeof member1;

          export const member2: unique symbol;
          export type member2 = typeof member2;
      }
      type SymbolEnum = typeof SymbolEnum[keyof typeof SymbolEnum];
      ```

      However as explained in https://github.com/microsoft/TypeScript/issues/43657:
      "A unique symbol type is never transferred from one declaration to another through inference."
      This intended behaviour in TS means that the usage of the fake-enum would look like this:
      ```
      const value: SymbolEnum.member1 = SymbolEnum.member1;
      //           ^^^^^^^^^^^^^^^^^^ required to force TS to retain the information
      ```
      Which is really clunky and shitty. It definitely works, but ofc it's not good.
      We can go with this design if users are okay with it!

      Considering how rarely used symbol enums are ATM, let's just put a pin in it for now.
      */
      return unsupportedDeclaration(
        node,
        'symbol enums',
        node.id,
        FlowESTree.isDeclareEnum(node),
      );
    }
    if (body.type === 'EnumBooleanBody') {
      /*
      TODO - TS enums only allow strings or numbers as their values - not booleans.
      This means we need a non-ts-enum representation of the enum.
      We can support boolean enums using a construct like this:
      ```ts
      declare namespace BooleanEnum {
          export const member1: true;
          export type member1 = typeof member1;

          export const member2: false;
          export type member2 = typeof member1;
      }
      declare type BooleanEnum = boolean;
      ```

      But it's pretty clunky and ugly.
      Considering how rarely used boolean enums are ATM, let's just put a pin in it for now.
      */
      return unsupportedDeclaration(
        node,
        'boolean enums',
        node.id,
        FlowESTree.isDeclareEnum(node),
      );
    }

    const members: Array<TSESTree.TSEnumMemberNonComputedName> = [];
    for (const member of body.members) {
      switch (member.type) {
        case 'EnumDefaultedMember': {
          if (body.type === 'EnumNumberBody') {
            // this should be impossible!
            throw unexpectedTranslationError(
              member,
              'Unexpected defaulted number enum member',
            );
          }
          members.push({
            type: 'TSEnumMember',
            computed: false,
            id: transform.Identifier(member.id, false),
            initializer: ({
              type: 'Literal',
              raw: `"${member.id.name}"`,
              value: member.id.name,
            }: TSESTree.StringLiteral),
          });
          break;
        }

        case 'EnumNumberMember':
        case 'EnumStringMember':
          members.push({
            type: 'TSEnumMember',
            computed: false,
            id: transform.Identifier(member.id, false),
            initializer:
              member.init.literalType === 'string'
                ? transform.StringLiteral(member.init)
                : transform.NumericLiteral(member.init),
          });
      }
    }

    const bodyRepresentationType =
      body.type === 'EnumNumberBody'
        ? {type: 'TSNumberKeyword'}
        : {type: 'TSStringKeyword'};

    const enumName = transform.Identifier(node.id, false);
    return [
      {
        type: 'TSEnumDeclaration',
        const: false,
        declare: true,
        id: enumName,
        members,
      },
      // flow also exports `.cast`, `.isValid`, `.members` and `.getName` for enums
      // we can use declaration merging to declare these functions on the enum:
      /*
      declare enum Foo {
        A = 1,
        B = 2,
      }
      declare namespace Foo {
        export function cast(value: number | null | undefined): Foo;
        export function isValid(value: number | null | undefined): value is Foo;
        export function members(): IterableIterator<Foo>;
        export function getName(value: Foo): string;
      }
      */
      {
        type: 'TSModuleDeclaration',
        declare: true,
        id: enumName,
        body: {
          type: 'TSModuleBlock',
          body: [
            // export function cast(value: number | null | undefined): Foo
            {
              type: 'ExportNamedDeclaration',
              declaration: {
                type: 'TSDeclareFunction',
                id: {
                  type: 'Identifier',
                  name: 'cast',
                },
                generator: false,
                expression: false,
                async: false,
                params: [
                  {
                    type: 'Identifier',
                    name: 'value',
                    typeAnnotation: {
                      type: 'TSTypeAnnotation',
                      typeAnnotation: {
                        type: 'TSUnionType',
                        types: [
                          bodyRepresentationType,
                          {
                            type: 'TSNullKeyword',
                          },
                          {
                            type: 'TSUndefinedKeyword',
                          },
                        ],
                      },
                    },
                  },
                ],
                returnType: {
                  type: 'TSTypeAnnotation',
                  typeAnnotation: {
                    type: 'TSTypeReference',
                    typeName: enumName,
                  },
                },
              },
              specifiers: [],
              source: null,
              exportKind: 'value',
              assertions: [],
            },
            // export function isValid(value: number | null | undefined): value is Foo;
            {
              type: 'ExportNamedDeclaration',
              declaration: {
                type: 'TSDeclareFunction',
                id: {
                  type: 'Identifier',
                  name: 'isValid',
                },
                generator: false,
                expression: false,
                async: false,
                params: [
                  {
                    type: 'Identifier',
                    name: 'value',
                    typeAnnotation: {
                      type: 'TSTypeAnnotation',
                      typeAnnotation: {
                        type: 'TSUnionType',
                        types: [
                          bodyRepresentationType,
                          {
                            type: 'TSNullKeyword',
                          },
                          {
                            type: 'TSUndefinedKeyword',
                          },
                        ],
                      },
                    },
                  },
                ],
                returnType: {
                  type: 'TSTypeAnnotation',
                  typeAnnotation: {
                    type: 'TSTypePredicate',
                    asserts: false,
                    parameterName: {
                      type: 'Identifier',
                      name: 'value',
                    },
                    typeAnnotation: {
                      type: 'TSTypeAnnotation',
                      typeAnnotation: {
                        type: 'TSTypeReference',
                        typeName: enumName,
                      },
                    },
                  },
                },
              },
              specifiers: [],
              source: null,
              exportKind: 'value',
              assertions: [],
            },
            // export function members(): IterableIterator<Foo>;
            {
              type: 'ExportNamedDeclaration',
              declaration: {
                type: 'TSDeclareFunction',
                id: {
                  type: 'Identifier',
                  name: 'members',
                },
                generator: false,
                expression: false,
                async: false,
                params: [],
                returnType: {
                  type: 'TSTypeAnnotation',
                  typeAnnotation: {
                    type: 'TSTypeReference',
                    typeName: {
                      type: 'Identifier',
                      name: 'IterableIterator',
                    },
                    typeParameters: {
                      type: 'TSTypeParameterInstantiation',
                      params: [
                        {
                          type: 'TSTypeReference',
                          typeName: enumName,
                        },
                      ],
                    },
                  },
                },
              },
              specifiers: [],
              source: null,
              exportKind: 'value',
              assertions: [],
            },
            // export function getName(value: Foo): string;
            {
              type: 'ExportNamedDeclaration',
              declaration: {
                type: 'TSDeclareFunction',
                id: {
                  type: 'Identifier',
                  name: 'getName',
                },
                generator: false,
                expression: false,
                async: false,
                params: [
                  {
                    type: 'Identifier',
                    name: 'value',
                    typeAnnotation: {
                      type: 'TSTypeAnnotation',
                      typeAnnotation: {
                        type: 'TSTypeReference',
                        typeName: enumName,
                      },
                    },
                  },
                ],
                returnType: {
                  type: 'TSTypeAnnotation',
                  typeAnnotation: {
                    type: 'TSStringKeyword',
                  },
                },
              },
              specifiers: [],
              source: null,
              exportKind: 'value',
              assertions: [],
            },
          ],
        },
      },
    ];
  }

  const getPlaceholderNameForTypeofImport: () => string = (() => {
    let typeof_import_count = 0;
    return () => `$$IMPORT_TYPEOF_${++typeof_import_count}$$`;
  })();

  const transformTypeAnnotationType = (
    node: FlowESTree.TypeAnnotationType,
  ): TSESTree.TypeNode => {
    switch (node.type) {
      case 'AnyTypeAnnotation':
        return transform.AnyTypeAnnotation(node);
      case 'ArrayTypeAnnotation':
        return transform.ArrayTypeAnnotation(node);
      case 'BigIntLiteralTypeAnnotation':
        return transform.BigIntLiteralTypeAnnotation(node);
      case 'BigIntTypeAnnotation':
        return transform.BigIntTypeAnnotation(node);
      case 'BooleanLiteralTypeAnnotation':
        return transform.BooleanLiteralTypeAnnotation(node);
      case 'BooleanTypeAnnotation':
        return transform.BooleanTypeAnnotation(node);
      case 'EmptyTypeAnnotation':
        return transform.EmptyTypeAnnotation(node);
      case 'ExistsTypeAnnotation':
        return transform.ExistsTypeAnnotation(node);
      case 'FunctionTypeAnnotation':
        return transform.FunctionTypeAnnotation(node);
      case 'GenericTypeAnnotation':
        return transform.GenericTypeAnnotation(node);
      case 'IndexedAccessType':
        return transform.IndexedAccessType(node);
      case 'InterfaceTypeAnnotation':
        return transform.InterfaceTypeAnnotation(node);
      case 'IntersectionTypeAnnotation':
        return transform.IntersectionTypeAnnotation(node);
      case 'MixedTypeAnnotation':
        return transform.MixedTypeAnnotation(node);
      case 'NullLiteralTypeAnnotation':
        return transform.NullLiteralTypeAnnotation(node);
      case 'NullableTypeAnnotation':
        return transform.NullableTypeAnnotation(node);
      case 'NumberLiteralTypeAnnotation':
        return transform.NumberLiteralTypeAnnotation(node);
      case 'NumberTypeAnnotation':
        return transform.NumberTypeAnnotation(node);
      case 'ObjectTypeAnnotation':
        return transform.ObjectTypeAnnotation(node);
      case 'OptionalIndexedAccessType':
        return transform.OptionalIndexedAccessType(node);
      case 'QualifiedTypeIdentifier':
        return transform.QualifiedTypeIdentifier(node);
      case 'StringLiteralTypeAnnotation':
        return transform.StringLiteralTypeAnnotation(node);
      case 'StringTypeAnnotation':
        return transform.StringTypeAnnotation(node);
      case 'SymbolTypeAnnotation':
        return transform.SymbolTypeAnnotation(node);
      case 'ThisTypeAnnotation':
        return transform.ThisTypeAnnotation(node);
      case 'TupleTypeAnnotation':
        return transform.TupleTypeAnnotation(node);
      case 'TypeofTypeAnnotation':
        return transform.TypeofTypeAnnotation(node);
      case 'UnionTypeAnnotation':
        return transform.UnionTypeAnnotation(node);
      case 'VoidTypeAnnotation':
        return transform.VoidTypeAnnotation(node);
      case 'TypePredicate':
        return transform.TypePredicateAnnotation(node);
      case 'ConditionalTypeAnnotation':
        return transform.ConditionalTypeAnnotation(node);
      case 'InferTypeAnnotation':
        return transform.InferTypeAnnotation(node);
      case 'KeyofTypeAnnotation':
        return transform.KeyofTypeAnnotation(node);
      default:
        throw unexpectedTranslationError(node, `Unhandled type ${node.type}`);
    }
  };

  const transform = {
    AnyTypeAnnotation(
      _node: FlowESTree.AnyTypeAnnotation,
    ): TSESTree.TSAnyKeyword {
      return {
        type: 'TSAnyKeyword',
      };
    },
    ArrayTypeAnnotation(
      node: FlowESTree.ArrayTypeAnnotation,
    ): TSESTree.TSArrayType {
      return {
        type: 'TSArrayType',
        elementType: transformTypeAnnotationType(node.elementType),
      };
    },
    BigIntLiteral(node: FlowESTree.BigIntLiteral): TSESTree.BigIntLiteral {
      return {
        type: 'Literal',
        bigint: node.bigint,
        raw: node.raw,
        value: node.value,
      };
    },
    BigIntLiteralTypeAnnotation(
      node: FlowESTree.BigIntLiteralTypeAnnotation,
    ): TSESTree.TSLiteralType {
      // technically hermes doesn't support this yet
      // but future proofing amirite
      const bigint =
        // $FlowExpectedError[prop-missing]
        node.bigint ??
        node.raw
          // estree spec is to not have a trailing `n` on this property
          // https://github.com/estree/estree/blob/db962bb417a97effcfe9892f87fbb93c81a68584/es2020.md#bigintliteral
          .replace(/n$/, '')
          // `BigInt` doesn't accept numeric separator and `bigint` property should not include numeric separator
          .replace(/_/, '');
      return {
        type: 'TSLiteralType',
        literal: ({
          type: 'Literal',
          value: node.value,
          raw: node.raw,
          bigint,
        }: TSESTree.BigIntLiteral),
      };
    },
    BigIntTypeAnnotation(
      _node: FlowESTree.BigIntTypeAnnotation,
    ): TSESTree.TSBigIntKeyword {
      return {
        type: 'TSBigIntKeyword',
      };
    },
    BooleanLiteral(node: FlowESTree.BooleanLiteral): TSESTree.BooleanLiteral {
      return {
        type: 'Literal',
        raw: node.raw,
        value: node.value,
      };
    },
    BooleanLiteralTypeAnnotation(
      node: FlowESTree.BooleanLiteralTypeAnnotation,
    ): TSESTree.TSLiteralType {
      return {
        type: 'TSLiteralType',
        literal: ({
          type: 'Literal',
          value: node.value,
          raw: node.raw,
        }: TSESTree.BooleanLiteral),
      };
    },
    BooleanTypeAnnotation(
      _node: FlowESTree.BooleanTypeAnnotation,
    ): TSESTree.TSBooleanKeyword {
      return {
        type: 'TSBooleanKeyword',
      };
    },
    ClassImplements(
      node: FlowESTree.ClassImplements,
    ): TSESTree.TSClassImplements {
      return {
        type: 'TSClassImplements',
        expression: transform.Identifier(node.id, false),
        typeParameters:
          node.typeParameters == null
            ? undefined
            : transform.TypeParameterInstantiation(node.typeParameters),
      };
    },
    DeclareClass(
      node: FlowESTree.DeclareClass,
    ): DeclarationOrUnsupported<TSESTree.ClassDeclarationWithName> {
      const classMembers: Array<TSESTree.ClassElement> = [];
      const transformedBody = transform.ObjectTypeAnnotation(node.body);
      if (transformedBody.type !== 'TSTypeLiteral') {
        return unsupportedDeclaration(
          node.body,
          'Spreads in declare class are not allowed',
          node.id,
          true,
          node.typeParameters,
        );
      }
      for (const member of transformedBody.members) {
        // TS uses the real ClassDeclaration AST so we need to
        // make the signatures real methods/properties
        switch (member.type) {
          case 'TSIndexSignature': {
            classMembers.push(member);
            break;
          }

          case 'TSMethodSignature': {
            // flow just creates a method signature like any other for a constructor
            // but the proper AST has `kind = 'constructor'`
            const isConstructor = (() => {
              if (member.computed === true) {
                return false;
              }

              return (
                (member.key.type === 'Identifier' &&
                  member.key.name === 'constructor') ||
                (member.key.type === 'Literal' &&
                  member.key.value === 'constructor')
              );
            })();
            if (isConstructor) {
              const newNode: TSESTree.MethodDefinitionAmbiguous = {
                type: 'MethodDefinition',
                accessibility: undefined,
                computed: false,
                key: {
                  type: 'Identifier',
                  name: 'constructor',
                },
                kind: 'constructor',
                optional: false,
                override: false,
                static: false,
                value: {
                  type: 'TSEmptyBodyFunctionExpression',
                  async: false,
                  body: null,
                  declare: false,
                  expression: false,
                  generator: false,
                  id: null,
                  params: member.params,
                  // constructors explicitly have no return type
                  returnType: undefined,
                  typeParameters: member.typeParameters,
                },
              };
              cloneJSDocCommentsToNewNode(member, newNode);
              classMembers.push(newNode);
            } else {
              const newNode: TSESTree.MethodDefinitionAmbiguous = {
                type: 'MethodDefinition',
                accessibility: member.accessibility,
                computed: member.computed ?? false,
                key: member.key,
                kind: member.kind,
                optional: member.optional,
                override: false,
                static: member.static ?? false,
                value: {
                  type: 'TSEmptyBodyFunctionExpression',
                  async: false,
                  body: null,
                  declare: false,
                  expression: false,
                  generator: false,
                  id: null,
                  params: member.params,
                  returnType: member.returnType,
                  typeParameters: member.typeParameters,
                },
              };
              cloneJSDocCommentsToNewNode(member, newNode);
              classMembers.push(newNode);
            }
            break;
          }

          case 'TSPropertySignature': {
            const newNode: TSESTree.PropertyDefinitionAmbiguous = {
              type: 'PropertyDefinition',
              accessibility: member.accessibility,
              computed: member.computed ?? false,
              declare: false,
              key: member.key,
              optional: member.optional,
              readonly: member.readonly,
              static: member.static ?? false,
              typeAnnotation: member.typeAnnotation,
              value: null,
            };
            cloneJSDocCommentsToNewNode(member, newNode);
            classMembers.push(newNode);
            break;
          }

          case 'TSCallSignatureDeclaration': {
            /*
            TODO - callProperties
            It's not valid to directly declare a call property on a class in TS
            You can do it, but it's a big complication in the AST:
            ```ts
            declare Class {
              // ...
            }
            interface ClassConstructor {
              new (): Class;
              // call sigs
              (): Type;
            }
            ```
            Let's put a pin in it for now and deal with it later if the need arises.
            */
            return unsupportedDeclaration(
              node.body.callProperties[0] ?? node.body,
              'call signatures on classes',
              node.id,
              true,
              node.typeParameters,
            );
          }

          default:
            throw unexpectedTranslationError(
              node.body,
              `Unexpected member type ${member.type}`,
            );
        }
      }

      const superClass = node.extends.length > 0 ? node.extends[0] : undefined;

      return {
        type: 'ClassDeclaration',
        body: {
          type: 'ClassBody',
          body: classMembers,
        },
        declare: true,
        id: transform.Identifier(node.id, false),
        implements:
          node.implements == null
            ? undefined
            : node.implements.map(transform.ClassImplements),
        superClass:
          superClass == null
            ? null
            : transform.Identifier(superClass.id, false),
        superTypeParameters:
          superClass?.typeParameters == null
            ? undefined
            : transform.TypeParameterInstantiation(superClass.typeParameters),
        typeParameters:
          node.typeParameters == null
            ? undefined
            : transform.TypeParameterDeclaration(node.typeParameters),
        // TODO - mixins??
      };
    },
    DeclareExportDeclaration(
      node: FlowESTree.DeclareExportDeclaration,
    ):
      | TSESTree.ExportNamedDeclaration
      | Array<TSESTree.ExportNamedDeclaration>
      | TSESTree.ExportDefaultDeclaration
      | [
          (
            | TSESTree.VariableDeclaration
            | TSESTree.ClassDeclaration
            | TSESTree.TSDeclareFunction
            | TSESTree.TSTypeAliasDeclaration
          ),
          TSESTree.ExportDefaultDeclaration,
        ] {
      if (node.default === true) {
        const declaration = node.declaration;

        switch (declaration.type) {
          // TS doesn't support direct default export for declare'd classes
          case 'DeclareClass': {
            const classDecl = transform.DeclareClass(declaration);
            const name = declaration.id.name;
            return [
              classDecl,
              {
                type: 'ExportDefaultDeclaration',
                declaration: {
                  type: 'Identifier',
                  name,
                },
                exportKind: 'value',
              },
            ];
          }

          // TS doesn't support direct default export for declare'd functions
          case 'DeclareFunction': {
            const functionDecl = transform.DeclareFunction(declaration);
            const name = declaration.id.name;
            return [
              functionDecl,
              {
                type: 'ExportDefaultDeclaration',
                declaration: {
                  type: 'Identifier',
                  name,
                },
                exportKind: 'value',
              },
            ];
          }

          // Flow's declare export default Identifier is ambiguous.
          // the Identifier might reference a type, or it might reference a value
          // - If it's a value, then that's all good, TS supports that.
          // - If it's a type, that's a problem - TS only allows value variables to be exported
          //   so we need to create an intermediate variable to hold the type.

          case 'GenericTypeAnnotation': {
            const referencedId = declaration.id;

            // QualifiedTypeIdentifiers are types so cannot be handled without the intermediate variable so
            // only Identifiers can be handled here.
            if (referencedId.type === 'Identifier') {
              const exportedVar = topScope.set.get(referencedId.name);
              if (exportedVar != null && exportedVar.defs.length === 1) {
                const def = exportedVar.defs[0];
                switch (def.type) {
                  case 'ImportBinding': {
                    // `import type { Wut } from 'mod'; declare export default Wut;`
                    // `import { type Wut } from 'mod'; declare export default Wut;`
                    // these cases should be wrapped in a variable because they're exporting a type, not a value
                    const specifier = def.node;
                    if (
                      specifier.importKind === 'type' ||
                      specifier.parent.importKind === 'type'
                    ) {
                      // fallthrough to the "default" handling
                      break;
                    }

                    // intentional fallthrough to the "value" handling
                  }
                  case 'ClassName':
                  case 'Enum':
                  case 'FunctionName':
                  case 'ImplicitGlobalVariable':
                  case 'Variable':
                    // there's already a variable defined to hold the type
                    return {
                      type: 'ExportDefaultDeclaration',
                      declaration: {
                        type: 'Identifier',
                        name: referencedId.name,
                      },
                      exportKind: 'value',
                    };

                  case 'CatchClause':
                  case 'Parameter':
                  case 'TypeParameter':
                    throw translationError(
                      def.node,
                      `Unexpected variable def type: ${def.type}`,
                    );

                  case 'Type':
                    // fallthrough to the "default" handling
                    break;
                }
              }
            }

            // intentional fallthrough to the "default" handling
          }

          default: {
            /*
            flow allows syntax like
            ```
            declare export default TypeName;
            ```
            but TS does not, so we have to declare a temporary variable to
            reference in the export declaration:
            ```
            declare const $$EXPORT_DEFAULT_DECLARATION$$: TypeName;
            export default $$EXPORT_DEFAULT_DECLARATION$$;
            ```
            */
            const SPECIFIER = '$$EXPORT_DEFAULT_DECLARATION$$';
            return [
              {
                type: 'VariableDeclaration',
                declarations: [
                  {
                    type: 'VariableDeclarator',
                    id: {
                      type: 'Identifier',
                      name: SPECIFIER,
                      typeAnnotation: {
                        type: 'TSTypeAnnotation',
                        typeAnnotation:
                          transformTypeAnnotationType(declaration),
                      },
                    },
                    init: null,
                  },
                ],
                declare: true,
                kind: 'const',
              },
              {
                type: 'ExportDefaultDeclaration',
                declaration: {
                  type: 'Identifier',
                  name: SPECIFIER,
                },
                exportKind: 'value',
              },
            ];
          }
        }
      } else {
        // eslint-disable-next-line eqeqeq
        if (node.source === null) {
          // eslint-disable-next-line eqeqeq
          if (node.declaration === null) {
            return ({
              type: 'ExportNamedDeclaration',
              // flow does not currently support assertions
              assertions: [],
              declaration: null,
              // flow does not support declared type exports with specifiers
              exportKind: 'value',
              source: null,
              specifiers: node.specifiers.map(transform.ExportSpecifier),
            }: TSESTree.ExportNamedDeclarationWithoutSourceWithMultiple);
          }

          const declarations = (() => {
            switch (node.declaration.type) {
              case 'DeclareClass':
                return [
                  {
                    declaration: transform.DeclareClass(node.declaration),
                    exportKind: 'value',
                  },
                ];
              case 'DeclareFunction':
                return [
                  {
                    declaration: transform.DeclareFunction(node.declaration),
                    exportKind: 'value',
                  },
                ];
              case 'DeclareInterface':
                return [
                  {
                    declaration: transform.DeclareInterface(node.declaration),
                    exportKind: 'type',
                  },
                ];
              case 'DeclareOpaqueType':
                return [
                  {
                    declaration: transform.DeclareOpaqueType(node.declaration),
                    exportKind: 'type',
                  },
                ];
              case 'DeclareVariable':
                return [
                  {
                    declaration: transform.DeclareVariable(node.declaration),
                    exportKind: 'value',
                  },
                ];
              case 'DeclareEnum': {
                const result = transform.DeclareEnum(node.declaration);
                return Array.isArray(result)
                  ? [
                      {
                        declaration: result[0],
                        exportKind: 'type',
                      },
                      {
                        declaration: result[1],
                        exportKind: 'type',
                      },
                    ]
                  : [{declaration: result, exportKind: 'type'}];
              }
            }
          })();

          return declarations.map(
            ({declaration, exportKind}) =>
              ({
                type: 'ExportNamedDeclaration',
                // flow does not currently support assertions
                assertions: [],
                declaration,
                exportKind,
                source: null,
                specifiers: [],
              }: TSESTree.ExportNamedDeclarationWithoutSourceWithSingle),
          );
        } else {
          return ({
            type: 'ExportNamedDeclaration',
            // flow does not currently support assertions
            assertions: [],
            declaration: null,
            // flow does not support declared type exports with a source
            exportKind: 'value',
            source: transform.StringLiteral(node.source),
            specifiers: node.specifiers.map(transform.ExportSpecifier),
          }: TSESTree.ExportNamedDeclarationWithSource);
        }
      }
    },
    DeclareFunction(
      node: FlowESTree.DeclareFunction,
    ): TSESTree.TSDeclareFunction {
      // the function information is stored as an annotation on the ID...
      const id = transform.Identifier(node.id, false);
      const functionInfo = transform.FunctionTypeAnnotation(
        node.id.typeAnnotation.typeAnnotation,
      );

      return {
        type: 'TSDeclareFunction',
        async: false,
        body: undefined,
        declare: true,
        expression: false,
        generator: false,
        id: {
          type: 'Identifier',
          name: id.name,
        },
        params: functionInfo.params,
        returnType: functionInfo.returnType,
        typeParameters: functionInfo.typeParameters,
      };
    },
    DeclareInterface(
      node: FlowESTree.DeclareInterface | FlowESTree.InterfaceDeclaration,
    ): TSESTree.TSInterfaceDeclaration {
      const transformedBody = transform.ObjectTypeAnnotation(node.body);
      if (transformedBody.type !== 'TSTypeLiteral') {
        throw translationError(
          node.body,
          'Spreads in interfaces are not allowed',
        );
      }

      return {
        type: 'TSInterfaceDeclaration',
        body: {
          type: 'TSInterfaceBody',
          body: transformedBody.members,
        },
        declare: node.type !== 'InterfaceDeclaration',
        extends: node.extends.map(transform.InterfaceExtends),
        id: transform.Identifier(node.id, false),
        typeParameters:
          node.typeParameters == null
            ? undefined
            : transform.TypeParameterDeclaration(node.typeParameters),
      };
    },
    DeclareTypeAlias(
      node: FlowESTree.DeclareTypeAlias | FlowESTree.TypeAlias,
    ): TSESTree.TSTypeAliasDeclaration {
      return {
        type: 'TSTypeAliasDeclaration',
        declare: node.type === 'DeclareTypeAlias',
        id: transform.Identifier(node.id, false),
        typeAnnotation: transformTypeAnnotationType(node.right),
        typeParameters:
          node.typeParameters == null
            ? undefined
            : transform.TypeParameterDeclaration(node.typeParameters),
      };
    },
    DeclareOpaqueType(
      node: FlowESTree.DeclareOpaqueType | FlowESTree.OpaqueType,
    ): TSESTree.TSTypeAliasDeclaration {
      // TS doesn't currently have nominal types - https://github.com/Microsoft/Typescript/issues/202
      // TODO - we could simulate this in a variety of ways
      // Examples - https://basarat.gitbook.io/typescript/main-1/nominaltyping

      return {
        type: 'TSTypeAliasDeclaration',
        declare: true,
        id: transform.Identifier(node.id, false),
        typeAnnotation:
          node.supertype == null
            ? {
                type: 'TSUnknownKeyword',
              }
            : transformTypeAnnotationType(node.supertype),
        typeParameters:
          node.typeParameters == null
            ? undefined
            : transform.TypeParameterDeclaration(node.typeParameters),
      };
    },
    DeclareVariable(
      node: FlowESTree.DeclareVariable,
    ): TSESTree.VariableDeclaration {
      return {
        type: 'VariableDeclaration',
        declare: true,
        declarations: [
          {
            type: 'VariableDeclarator',
            declare: true,
            id: transform.Identifier(node.id, true),
            init: null,
          },
        ],
        kind: node.kind,
      };
    },
    DeclareEnum(
      node: FlowESTree.DeclareEnum,
    ): DeclarationOrUnsupported<
      [TSESTree.TSEnumDeclaration, TSESTree.TSModuleDeclaration],
    > {
      return EnumImpl(node);
    },
    EmptyTypeAnnotation(
      node: FlowESTree.EmptyTypeAnnotation,
    ): TSESTree.TypeNode {
      // Flow's `empty` type doesn't map well to any types in TS.
      // The closest is `never`, but `never` has a number of different semantics
      // In reality no human code should ever directly use the `empty` type in flow
      // So let's put a pin in it for now
      return unsupportedAnnotation(node, 'empty type');
    },
    EnumDeclaration(
      node: FlowESTree.EnumDeclaration,
    ): DeclarationOrUnsupported<
      [TSESTree.TSEnumDeclaration, TSESTree.TSModuleDeclaration],
    > {
      return EnumImpl(node);
    },
    DeclareModuleExports(
      node: FlowESTree.DeclareModuleExports,
    ): TSESTree.TypeNode {
      throw translationError(node, 'CommonJS exports are not supported.');
    },
    ExistsTypeAnnotation(
      node: FlowESTree.ExistsTypeAnnotation,
    ): TSESTree.TypeNode {
      // The existential type does not map to any types in TS
      // It's also super deprecated - so let's not ever worry
      return unsupportedAnnotation(node, 'existential type');
    },
    ExportAllDeclaration(
      node: FlowESTree.ExportAllDeclaration,
    ): TSESTree.ExportAllDeclaration {
      return {
        type: 'ExportAllDeclaration',
        // flow does not currently support import/export assertions
        assertions: [],
        exportKind: node.exportKind,
        source: transform.StringLiteral(node.source),
        exported:
          node.exported == null ? null : transform.Identifier(node.exported),
      };
    },
    ExportNamedDeclaration(
      node: FlowESTree.ExportNamedDeclaration,
    ):
      | TSESTree.ExportNamedDeclarationAmbiguous
      | [TSESTree.ExportNamedDeclarationAmbiguous, ?TSESTree.Node] {
      if (node.source != null || node.specifiers.length > 0) {
        // can never have a declaration with a source
        return {
          type: 'ExportNamedDeclaration',
          // flow does not currently support import/export assertions
          assertions: [],
          declaration: null,
          exportKind: node.exportKind,
          source:
            node.source == null ? null : transform.StringLiteral(node.source),
          specifiers: node.specifiers.map(transform.ExportSpecifier),
        };
      }

      const [exportedDeclaration, mergedDeclaration] = (() => {
        if (node.declaration == null) {
          return [null, null];
        }

        switch (node.declaration.type) {
          case 'ClassDeclaration':
          case 'ComponentDeclaration':
          case 'FunctionDeclaration':
          case 'VariableDeclaration':
            // These cases shouldn't happen in flow defs because they have their own special
            // AST node (DeclareClass, DeclareFunction, DeclareVariable)
            throw unexpectedTranslationError(
              node.declaration,
              `Unexpected named declaration found ${node.declaration.type}`,
            );

          case 'EnumDeclaration': {
            const result = transform.EnumDeclaration(node.declaration);
            return Array.isArray(result) ? result : [result, null];
          }
          case 'InterfaceDeclaration':
            return [transform.InterfaceDeclaration(node.declaration), null];
          case 'OpaqueType':
            return [transform.OpaqueType(node.declaration), null];
          case 'TypeAlias':
            return [transform.TypeAlias(node.declaration), null];
        }
      })();

      const mainExport = {
        type: 'ExportNamedDeclaration',
        assertions: [],
        declaration: exportedDeclaration,
        exportKind: node.exportKind,
        source: null,
        specifiers: [],
      };

      if (mergedDeclaration != null) {
        // for cases where there is a merged declaration, TS enforces BOTH are exported
        return [
          mainExport,
          {
            type: 'ExportNamedDeclaration',
            assertions: [],
            declaration: mergedDeclaration,
            exportKind: node.exportKind,
            source: null,
            specifiers: [],
          },
        ];
      }

      return mainExport;
    },
    ExportSpecifier(
      node: FlowESTree.ExportSpecifier,
    ): TSESTree.ExportSpecifier {
      return {
        type: 'ExportSpecifier',
        exported: transform.Identifier(node.exported, false),
        local: transform.Identifier(node.local, false),
        // flow does not support inline exportKind for named exports
        exportKind: 'value',
      };
    },
    FunctionTypeAnnotation(
      node: FlowESTree.FunctionTypeAnnotation,
    ): TSESTree.TSFunctionType {
      const params = node.params.map(transform.FunctionTypeParam);
      if (node.this != null) {
        params.unshift({
          type: 'Identifier',
          name: 'this',
          typeAnnotation: {
            type: 'TSTypeAnnotation',
            typeAnnotation: transformTypeAnnotationType(
              node.this.typeAnnotation,
            ),
          },
        });
      }
      if (node.rest != null) {
        const rest = node.rest;
        params.push({
          type: 'RestElement',
          argument:
            rest.name == null
              ? {
                  type: 'Identifier',
                  name: '$$REST$$',
                }
              : transform.Identifier(rest.name, false),
          typeAnnotation: {
            type: 'TSTypeAnnotation',
            typeAnnotation: transformTypeAnnotationType(rest.typeAnnotation),
          },
        });
      }

      return {
        type: 'TSFunctionType',
        params,
        returnType: {
          type: 'TSTypeAnnotation',
          typeAnnotation: transformTypeAnnotationType(node.returnType),
        },
        typeParameters:
          node.typeParameters == null
            ? undefined
            : transform.TypeParameterDeclaration(node.typeParameters),
      };
    },
    FunctionTypeParam(
      node: FlowESTree.FunctionTypeParam,
      idx: number = 0,
    ): TSESTree.Parameter {
      return {
        type: 'Identifier',
        name: node.name == null ? `$$PARAM_${idx}$$` : node.name.name,
        typeAnnotation: {
          type: 'TSTypeAnnotation',
          typeAnnotation: transformTypeAnnotationType(node.typeAnnotation),
        },
        optional: node.optional,
      };
    },
    GenericTypeAnnotation(
      node: FlowESTree.GenericTypeAnnotation,
    ): TSESTree.TypeNode {
      const [fullTypeName, baseId] = (() => {
        let names: Array<string> = [];
        let currentNode = node.id;

        while (currentNode != null) {
          switch (currentNode.type) {
            case 'Identifier': {
              names.unshift(currentNode.name);
              return [names.join('.'), currentNode];
            }
            case 'QualifiedTypeIdentifier': {
              names.unshift(currentNode.id.name);
              currentNode = currentNode.qualification;
              break;
            }
          }
        }

        throw translationError(
          node,
          `Invalid program state, types should only contain 'Identifier' and 'QualifiedTypeIdentifier' nodes.`,
        );
      })();

      const assertHasExactlyNTypeParameters = (
        count: number,
      ): $ReadOnlyArray<TSESTree.TypeNode> => {
        if (node.typeParameters != null) {
          if (node.typeParameters.params.length !== count) {
            throw translationError(
              node,
              `Expected exactly ${count} type parameter${
                count > 1 ? 's' : ''
              } with \`${fullTypeName}\``,
            );
          }

          const res = [];
          for (const param of node.typeParameters.params) {
            res.push(transformTypeAnnotationType(param));
          }
          return res;
        }

        if (count !== 0) {
          throw translationError(
            node,
            `Expected no type parameters with \`${fullTypeName}\``,
          );
        }

        return [];
      };

      switch (fullTypeName) {
        case '$Call':
        case '$ObjMap':
        case '$ObjMapConst':
        case '$ObjMapi':
        case '$TupleMap': {
          /*
          TODO - I don't think it's possible to make these types work in the generic case.

          TS has no utility types that allow you to generically mimic this functionality.
          You really need intimiate knowledge of the user's intent in order to correctly
          transform the code.

          For example the simple example for $Call from the flow docs:
          ```
          type ExtractPropType = <T>({prop: T}) => T;
          type Obj = {prop: number};
          type PropType = $Call<ExtractPropType, Obj>;
          // expected -- typeof PropType === number
          ```

          The equivalent in TS would be:
          ```
          type ExtractPropType<T extends { prop: any }> = (arg: T) => T['prop'];
          type Obj = { prop: number };
          type PropType = ReturnType<ExtractPropType<Obj>>; // number
          ```
          */
          return unsupportedAnnotation(node, fullTypeName);
        }

        case '$Diff':
        case '$Rest': {
          // `$Diff<A, B>` => `Pick<A, Exclude<keyof A, keyof B>>`
          const params = assertHasExactlyNTypeParameters(2);
          return {
            type: 'TSTypeReference',
            typeName: {
              type: 'Identifier',
              name: 'Pick',
            },
            typeParameters: {
              type: 'TSTypeParameterInstantiation',
              params: [
                params[0],
                {
                  type: 'TSTypeReference',
                  typeName: {
                    type: 'Identifier',
                    name: 'Exclude',
                  },
                  typeParameters: {
                    type: 'TSTypeParameterInstantiation',
                    params: [
                      {
                        type: 'TSTypeOperator',
                        operator: 'keyof',
                        typeAnnotation: params[0],
                      },
                      {
                        type: 'TSTypeOperator',
                        operator: 'keyof',
                        typeAnnotation: params[1],
                      },
                    ],
                  },
                },
              ],
            },
          };
        }

        case '$ElementType':
        case '$PropertyType': {
          // `$ElementType<T, K>` => `T[K]`
          const params = assertHasExactlyNTypeParameters(2);
          return {
            type: 'TSIndexedAccessType',
            objectType: params[0],
            indexType: params[1],
          };
        }

        case '$Exact': {
          // `$Exact<T>` => `T`
          // TS has no concept of exact vs inexact types
          return assertHasExactlyNTypeParameters(1)[0];
        }

        case '$Exports': {
          // `$Exports<'module'>` => `typeof import('module')`
          const moduleName = assertHasExactlyNTypeParameters(1)[0];
          if (
            moduleName.type !== 'TSLiteralType' ||
            moduleName.literal.type !== 'Literal' ||
            typeof moduleName.literal.value !== 'string'
          ) {
            throw translationError(
              node,
              '$Exports must have a string literal argument',
            );
          }

          return {
            type: 'TSImportType',
            isTypeOf: true,
            argument: moduleName,
            qualifier: null,
            typeParameters: null,
          };
        }

        case '$FlowFixMe': {
          // `$FlowFixMe` => `any`
          return {
            type: 'TSAnyKeyword',
          };
        }

        case '$KeyMirror': {
          // `$KeyMirror<T>` => `{[K in keyof T]: K}`
          return {
            type: 'TSMappedType',
            typeParameter: {
              type: 'TSTypeParameter',
              name: {
                type: 'Identifier',
                name: 'K',
              },
              constraint: {
                type: 'TSTypeOperator',
                operator: 'keyof',
                typeAnnotation: assertHasExactlyNTypeParameters(1)[0],
              },
              in: false,
              out: false,
            },
            nameType: null,
            typeAnnotation: {
              type: 'TSTypeReference',
              typeName: {
                type: 'Identifier',
                name: 'K',
              },
            },
          };
        }

        case '$Keys': {
          // `$Keys<T>` => `keyof T`
          return {
            type: 'TSTypeOperator',
            operator: 'keyof',
            typeAnnotation: assertHasExactlyNTypeParameters(1)[0],
          };
        }

        case '$NonMaybeType': {
          // `$NonMaybeType<T>` => `NonNullable<T>`
          // Not a great name because `NonNullable` also excludes `undefined`
          return {
            type: 'TSTypeReference',
            typeName: {
              type: 'Identifier',
              name: 'NonNullable',
            },
            typeParameters: {
              type: 'TSTypeParameterInstantiation',
              params: assertHasExactlyNTypeParameters(1),
            },
          };
        }

        case '$ReadOnly': {
          // `$ReadOnly<T>` => `Readonly<T>`
          return {
            type: 'TSTypeReference',
            typeName: {
              type: 'Identifier',
              name: 'Readonly',
            },
            typeParameters: {
              type: 'TSTypeParameterInstantiation',
              params: assertHasExactlyNTypeParameters(1),
            },
          };
        }

        case '$ReadOnlyArray': {
          // `$ReadOnlyArray<T>` => `ReadonlyArray<T>`
          //
          // we could also do    => `readonly T[]`
          // TODO - maybe a config option?
          return {
            type: 'TSTypeReference',
            typeName: {
              type: 'Identifier',
              name: 'ReadonlyArray',
            },
            typeParameters: {
              type: 'TSTypeParameterInstantiation',
              params: assertHasExactlyNTypeParameters(1),
            },
          };
        }

        case '$ReadOnlyMap': {
          return {
            type: 'TSTypeReference',
            typeName: {
              type: 'Identifier',
              name: 'ReadonlyMap',
            },
            typeParameters: {
              type: 'TSTypeParameterInstantiation',
              params: assertHasExactlyNTypeParameters(1),
            },
          };
        }

        case '$ReadOnlySet': {
          return {
            type: 'TSTypeReference',
            typeName: {
              type: 'Identifier',
              name: 'ReadonlySet',
            },
            typeParameters: {
              type: 'TSTypeParameterInstantiation',
              params: assertHasExactlyNTypeParameters(1),
            },
          };
        }

        case '$Values': {
          // `$Values<T>` => `T[keyof T]`
          const transformedType = assertHasExactlyNTypeParameters(1)[0];
          return {
            type: 'TSIndexedAccessType',
            objectType: transformedType,
            indexType: {
              type: 'TSTypeOperator',
              operator: 'keyof',
              typeAnnotation: transformedType,
            },
          };
        }

        case 'Class': {
          // `Class<T>` => `new (...args: any[]) => T`
          const param = assertHasExactlyNTypeParameters(1)[0];
          if (param.type !== 'TSTypeReference') {
            throw translationError(
              node,
              'Expected a type reference within Class<T>',
            );
          }

          return {
            type: 'TSConstructorType',
            abstract: false,
            params: [
              {
                type: 'RestElement',
                argument: {
                  type: 'Identifier',
                  name: 'args',
                },
                typeAnnotation: {
                  type: 'TSTypeAnnotation',
                  typeAnnotation: {
                    type: 'TSArrayType',
                    elementType: {
                      type: 'TSAnyKeyword',
                    },
                  },
                },
              },
            ],
            returnType: {
              type: 'TSTypeAnnotation',
              typeAnnotation: param,
            },
          };
        }
      }

      // React special conversion:
      const validReactImportOrGlobal = isValidReactImportOrGlobal(baseId);
      const reactImport = isReactImport(baseId);
      if (validReactImportOrGlobal || reactImport) {
        // Returns appropriate Identifier for `React` import.
        // If a global is in use, set a flag to indicate that we should add the import.
        const getReactIdentifier = () => {
          if (shouldAddReactImport !== false) {
            shouldAddReactImport = !reactImport;
          }

          return {
            type: 'Identifier',
            name: `React`,
          };
        };

        switch (fullTypeName) {
          // TODO: In flow this is `ChildrenArray<T> = T | $ReadOnlyArray<ChildrenArray<T>>`.
          // The recursive nature of it is rarely needed, so we're simplifying this for now
          // but omitting that aspect. Once we're able to provide utility types for our translations,
          // we should update this.
          // React.ChildrenArray<T> -> T | ReadonlyArray<T>
          // React$ChildrenArray<T> -> T | ReadonlyArray<T>
          case 'React.ChildrenArray':
          case 'React$ChildrenArray': {
            const [param] = assertHasExactlyNTypeParameters(1);
            return {
              type: 'TSUnionType',
              types: [
                param,
                {
                  type: 'TSTypeReference',
                  typeName: {
                    type: 'Identifier',
                    name: 'ReadonlyArray',
                  },
                  typeParameters: {
                    type: 'TSTypeParameterInstantiation',
                    params: [param],
                  },
                },
              ],
            };
          }
          // React.Component<A,B> -> React.Component<A,B>
          // React$Component<A,B> -> React.Component<A,B>
          case 'React.Component':
          case 'React$Component': {
            const typeParameters = node.typeParameters;
            if (typeParameters == null || typeParameters.params.length === 0) {
              throw translationError(
                node,
                `Expected at least 1 type parameter with \`${fullTypeName}\``,
              );
            }
            const params = typeParameters.params;
            if (params.length > 2) {
              throw translationError(
                node,
                `Expected at no more than 2 type parameters with \`${fullTypeName}\``,
              );
            }

            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: 'Component',
                },
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: params.map(param => transformTypeAnnotationType(param)),
              },
            };
          }

          // React.Context<A> -> React.Context<A>
          // React$Context<A> -> React.Context<A>
          case 'React$Context':
          case 'React.Context':
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: `Context`,
                },
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: assertHasExactlyNTypeParameters(1),
              },
            };
          // React.Key -> React.Key
          // React$Key -> React.Key
          case 'React.Key':
          case 'React$Key':
            assertHasExactlyNTypeParameters(0);
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: 'Key',
                },
              },
            };
          // React.ElementType -> React.ElementType
          // React$ElementType -> React.ElementType
          case 'React$ElementType':
          case 'React.ElementType': {
            assertHasExactlyNTypeParameters(0);
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: `ElementType`,
                },
              },
              typeParameters: undefined,
            };
          }
          // React.Node -> React.ReactNode
          case 'React$Node':
          case 'React.Node': {
            assertHasExactlyNTypeParameters(0);
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: `ReactNode`,
                },
              },
              typeParameters: undefined,
            };
          }
          // React.Element<typeof Component> -> React.ReactElement<typeof Component>
          case 'React$Element':
          case 'React.Element': {
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: `ReactElement`,
                },
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: assertHasExactlyNTypeParameters(1),
              },
            };
          }
          // React.ElementRef<typeof Component> -> React.ElementRef<typeof Component>
          // React$ElementRef<typeof Component> -> React.ElementRef<typeof Component>
          case 'React$ElementRef':
          case 'React.ElementRef':
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: `ElementRef`,
                },
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: assertHasExactlyNTypeParameters(1),
              },
            };
          // React$Fragment -> React.Fragment
          // React.Fragment -> React.Fragment
          case 'React$FragmentType':
          case 'React.Fragment':
            assertHasExactlyNTypeParameters(0);
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: `Fragment`,
                },
              },
            };
          // React.MixedElement -> JSX.Element
          case 'React$MixedElement':
          case 'React.MixedElement': {
            assertHasExactlyNTypeParameters(0);
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: {
                  type: 'Identifier',
                  name: 'JSX',
                },
                right: {
                  type: 'Identifier',
                  name: 'Element',
                },
              },
              typeParameters: undefined,
            };
          }
          // React.ComponentType<Config> -> React.ComponentType<Config>
          // React$ComponentType<Config> -> React.ComponentType<Config>
          case 'React.ComponentType':
          case 'React$ComponentType': {
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: 'ComponentType',
                },
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: assertHasExactlyNTypeParameters(1),
              },
            };
          }
          // React.AbstractComponent<Config> -> React.ComponentType<Config>
          // React$AbstractComponent<Config> -> React.ComponentType<Config>
          // React.AbstractComponent<Config, Instance> -> React.ComponentType<Config & React.RefAttributes<Instance>>
          // React$AbstractComponent<Config, Instance> -> React.ComponentType<Config & React.RefAttributes<Instance>>
          case 'React.AbstractComponent':
          case 'React$AbstractComponent': {
            const typeParameters = node.typeParameters;
            if (typeParameters == null || typeParameters.params.length === 0) {
              throw translationError(
                node,
                `Expected at least 1 type parameter with \`${fullTypeName}\``,
              );
            }
            const params = typeParameters.params;
            if (params.length > 2) {
              throw translationError(
                node,
                `Expected at no more than 2 type parameters with \`${fullTypeName}\``,
              );
            }

            const newParams = (() => {
              if (params.length === 1) {
                return assertHasExactlyNTypeParameters(1);
              }

              const [props, ref] = assertHasExactlyNTypeParameters(2);

              return [
                {
                  type: 'TSIntersectionType',
                  types: [
                    props,
                    {
                      type: 'TSTypeReference',
                      typeName: {
                        type: 'TSQualifiedName',
                        left: {
                          type: 'Identifier',
                          name: 'React',
                        },
                        right: {
                          type: 'Identifier',
                          name: 'RefAttributes',
                        },
                      },
                      typeParameters: {
                        type: 'TSTypeParameterInstantiation',
                        params: [ref],
                      },
                    },
                  ],
                },
              ];
            })();

            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: 'ComponentType',
                },
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: newParams,
              },
            };
          }
          // React.ElementProps<A> -> React.ComponentProps<A>
          // React$ElementProps<A> -> React.ComponentProps<A>
          case 'React.ElementProps':
          case 'React$ElementProps': {
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: getReactIdentifier(),
                right: {
                  type: 'Identifier',
                  name: 'ComponentProps',
                },
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: assertHasExactlyNTypeParameters(1),
              },
            };
          }
          // React.ElementConfig<A> ->  JSX.LibraryManagedAttributes<A, React.ComponentProps<A>>
          // React$ElementConfig<A> ->  JSX.LibraryManagedAttributes<A, React.ComponentProps<A>>
          case 'React.ElementConfig':
          case 'React$ElementConfig': {
            const [param] = assertHasExactlyNTypeParameters(1);
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'TSQualifiedName',
                left: {
                  type: 'Identifier',
                  name: 'JSX',
                },
                right: {
                  type: 'Identifier',
                  name: 'LibraryManagedAttributes',
                },
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: [
                  param,
                  {
                    type: 'TSTypeReference',
                    typeName: {
                      type: 'TSQualifiedName',
                      left: getReactIdentifier(),
                      right: {
                        type: 'Identifier',
                        name: `ComponentProps`,
                      },
                    },
                    typeParameters: {
                      type: 'TSTypeParameterInstantiation',
                      params: [param],
                    },
                  },
                ],
              },
            };
          }
          // React.Ref<C> -> NonNullable<React.Ref<C> | string | number>
          // React$Ref<C> -> NonNullable<React.Ref<C> | string | number>
          case 'React.Ref':
          case 'React$Ref':
            return {
              type: 'TSTypeReference',
              typeName: {
                type: 'Identifier',
                name: 'NonNullable',
              },
              typeParameters: {
                type: 'TSTypeParameterInstantiation',
                params: [
                  {
                    type: 'TSUnionType',
                    types: [
                      {
                        type: 'TSTypeReference',
                        typeName: {
                          type: 'TSQualifiedName',
                          left: getReactIdentifier(),
                          right: {
                            type: 'Identifier',
                            name: 'Ref',
                          },
                        },
                        typeParameters: {
                          type: 'TSTypeParameterInstantiation',
                          params: assertHasExactlyNTypeParameters(1),
                        },
                      },
                      {
                        type: 'TSStringKeyword',
                      },
                      {
                        type: 'TSNumberKeyword',
                      },
                    ],
                  },
                ],
              },
            };
          default:
            return unsupportedAnnotation(node, fullTypeName);
        }
      }

      return {
        type: 'TSTypeReference',
        typeName:
          node.id.type === 'Identifier'
            ? transform.Identifier(node.id, false)
            : transform.QualifiedTypeIdentifier(node.id),
        typeParameters:
          node.typeParameters == null
            ? undefined
            : transform.TypeParameterInstantiation(node.typeParameters),
      };
    },
    Identifier(
      node: FlowESTree.Identifier,
      includeTypeAnnotation: boolean = true,
    ): TSESTree.Identifier {
      return {
        type: 'Identifier',
        name: node.name,
        ...(includeTypeAnnotation && node.typeAnnotation != null
          ? {
              typeAnnotation: transform.TypeAnnotation(node.typeAnnotation),
            }
          : {}),
      };
    },
    IndexedAccessType(
      node: FlowESTree.IndexedAccessType | FlowESTree.OptionalIndexedAccessType,
    ): TSESTree.TSIndexedAccessType {
      return {
        type: 'TSIndexedAccessType',
        objectType: transformTypeAnnotationType(node.objectType),
        indexType: transformTypeAnnotationType(node.indexType),
      };
    },
    InterfaceDeclaration(
      node: FlowESTree.InterfaceDeclaration,
    ): TSESTree.TSInterfaceDeclaration {
      return transform.DeclareInterface(node);
    },
    ImportAttribute(
      node: FlowESTree.ImportAttribute,
    ): TSESTree.ImportAttribute {
      return {
        type: 'ImportAttribute',
        key:
          node.key.type === 'Identifier'
            ? transform.Identifier(node.key)
            : transform.Literal(node.key),
        value: transform.Literal(node.value),
      };
    },
    ImportDeclaration(
      node: FlowESTree.ImportDeclaration,
    ): Array<DeclarationOrUnsupported<TSESTree.ImportDeclaration>> {
      const importKind = node.importKind;

      const specifiers = [];
      const unsupportedSpecifiers = [];
      node.specifiers.forEach(spec => {
        let id = (() => {
          if (node.importKind === 'typeof' || spec.importKind === 'typeof') {
            const id = {
              type: 'Identifier',
              name: getPlaceholderNameForTypeofImport(),
            };

            unsupportedSpecifiers.push({
              type: 'TSTypeAliasDeclaration',
              id: transform.Identifier(spec.local, false),
              typeAnnotation: {
                type: 'TSTypeQuery',
                exprName: id,
              },
            });

            return id;
          }

          return transform.Identifier(spec.local, false);
        })();

        switch (spec.type) {
          case 'ImportDefaultSpecifier':
            specifiers.push({
              type: 'ImportDefaultSpecifier',
              local: id,
            });
            return;

          case 'ImportNamespaceSpecifier':
            specifiers.push({
              type: 'ImportNamespaceSpecifier',
              local: id,
            });
            return;

          case 'ImportSpecifier':
            specifiers.push({
              type: 'ImportSpecifier',
              importKind:
                spec.importKind === 'typeof' || spec.importKind === 'type'
                  ? 'type'
                  : null,
              imported: transform.Identifier(spec.imported, false),
              local: id,
            });
            return;
        }
      });

      const out = specifiers.length
        ? [
            {
              type: 'ImportDeclaration',
              assertions: node.assertions.map(transform.ImportAttribute),
              importKind:
                importKind === 'typeof' ? 'type' : importKind ?? 'value',
              source: transform.StringLiteral(node.source),
              specifiers,
            },
          ]
        : [];

      return [...out, ...unsupportedSpecifiers];
    },
    InterfaceExtends(
      node: FlowESTree.InterfaceExtends,
    ): TSESTree.TSInterfaceHeritage {
      return {
        type: 'TSInterfaceHeritage',
        expression: transform.Identifier(node.id, false),
        typeParameters:
          node.typeParameters == null
            ? undefined
            : transform.TypeParameterInstantiation(node.typeParameters),
      };
    },
    InterfaceTypeAnnotation(
      node: FlowESTree.InterfaceTypeAnnotation,
    ): TSESTree.TypeNode {
      if (node.extends) {
        // type T = interface extends U, V { ... }
        // to
        // type T = U & V & { ... }
        return {
          type: 'TSIntersectionType',
          types: [
            ...node.extends.map(ex => ({
              type: 'TSTypeReference',
              typeName: transform.Identifier(ex.id, false),
              typeParameters:
                ex.typeParameters == null
                  ? undefined
                  : transform.TypeParameterInstantiation(ex.typeParameters),
            })),
            transform.ObjectTypeAnnotation(node.body),
          ],
        };
      }

      return transform.ObjectTypeAnnotation(node.body);
    },
    IntersectionTypeAnnotation(
      node: FlowESTree.IntersectionTypeAnnotation,
    ): TSESTree.TSIntersectionType {
      return {
        type: 'TSIntersectionType',
        types: node.types.map(transformTypeAnnotationType),
      };
    },
    Literal(node: FlowESTree.Literal): TSESTree.Literal {
      switch (node.literalType) {
        case 'bigint':
          return transform.BigIntLiteral(node);
        case 'boolean':
          return transform.BooleanLiteral(node);
        case 'null':
          return transform.NullLiteral(node);
        case 'numeric':
          return transform.NumericLiteral(node);
        case 'regexp':
          return transform.RegExpLiteral(node);
        case 'string':
          return transform.StringLiteral(node);
      }
    },
    MixedTypeAnnotation(
      _node: FlowESTree.MixedTypeAnnotation,
    ): TSESTree.TSUnknownKeyword {
      return {
        type: 'TSUnknownKeyword',
      };
    },
    NullLiteral(_node: FlowESTree.NullLiteral): TSESTree.NullLiteral {
      return {
        type: 'Literal',
        raw: 'null',
        value: null,
      };
    },
    NullLiteralTypeAnnotation(
      _node: FlowESTree.NullLiteralTypeAnnotation,
    ): TSESTree.TSNullKeyword {
      return {
        type: 'TSNullKeyword',
      };
    },
    NullableTypeAnnotation(
      node: FlowESTree.NullableTypeAnnotation,
    ): TSESTree.TSUnionType {
      // TS doesn't support the maybe type, so have to explicitly union in `null | undefined`
      // `?T` becomes `null | undefined | T`
      return {
        type: 'TSUnionType',
        types: [
          {
            type: 'TSNullKeyword',
          },
          {
            type: 'TSUndefinedKeyword',
          },
          transformTypeAnnotationType(node.typeAnnotation),
        ],
      };
    },
    NumberLiteralTypeAnnotation(
      node: FlowESTree.NumberLiteralTypeAnnotation,
    ): TSESTree.TSLiteralType {
      return {
        type: 'TSLiteralType',
        literal: ({
          type: 'Literal',
          value: node.value,
          raw: node.raw,
        }: TSESTree.NumberLiteral),
      };
    },
    NumberTypeAnnotation(
      _node: FlowESTree.NumberTypeAnnotation,
    ): TSESTree.TSNumberKeyword {
      return {
        type: 'TSNumberKeyword',
      };
    },
    NumericLiteral(node: FlowESTree.NumericLiteral): TSESTree.NumberLiteral {
      return {
        type: 'Literal',
        raw: node.raw,
        value: node.value,
      };
    },
    ObjectTypeAnnotation(
      node: FlowESTree.ObjectTypeAnnotation,
    ):
      | TSESTree.TSTypeLiteral
      | TSESTree.TSIntersectionType
      | TSESTree.TSAnyKeyword
      | TSESTree.TSMappedType {
      if (
        node.properties.length === 1 &&
        node.properties[0].type === 'ObjectTypeMappedTypeProperty'
      ) {
        // Mapped Object Object types must not have other object properties.
        const prop: FlowESTree.ObjectTypeMappedTypeProperty =
          node.properties[0];
        const tsProp: TSESTree.TSMappedType = {
          type: 'TSMappedType',
          typeParameter: {
            type: 'TSTypeParameter',
            name: {
              type: 'Identifier',
              name: prop.keyTparam.name,
            },
            constraint: transformTypeAnnotationType(prop.sourceType),
            in: false,
            out: false,
          },
          readonly: prop.variance?.kind === 'plus',
          optional: prop.optional === 'Optional',
          typeAnnotation: transformTypeAnnotationType(prop.propType),
          nameType: null,
        };

        return tsProp;
      }

      // we want to preserve the source order of the members
      // unfortunately flow has unordered properties storing things
      // so store all elements with their start index and sort the
      // list afterward
      const members: Array<{start: number, node: TSESTree.TypeElement}> = [];

      for (const callProp of node.callProperties) {
        members.push({
          start: callProp.range[0],
          node: transform.ObjectTypeCallProperty(callProp),
        });
      }

      for (const indexer of node.indexers) {
        members.push({
          start: indexer.range[0],
          node: transform.ObjectTypeIndexer(indexer),
        });
      }

      /*
      TODO - internalSlots
      I don't think there's anything analogous in TS.
      They're really rarely used (if ever) - so let's just ignore them for now
      */
      if (node.internalSlots.length > 0) {
        return unsupportedAnnotation(node.internalSlots[0], 'internal slots');
      }

      if (!node.properties.find(FlowESTree.isObjectTypeSpreadProperty)) {
        for (const property of node.properties) {
          if (property.type === 'ObjectTypeSpreadProperty') {
            // this is imposible due to the above find condition
            // this check is purely to satisfy flow
            throw unexpectedTranslationError(property, 'Impossible state');
          }

          if (property.type === 'ObjectTypeMappedTypeProperty') {
            return unsupportedAnnotation(
              property,
              'object type with mapped type property along with other properties',
            );
          }

          members.push({
            start: property.range[0],
            node: transform.ObjectTypeProperty(property),
          });
        }

        const tsBody = members
          .sort((a, b) => a.start - b.start)
          .map(({node}) => node);

        return {
          type: 'TSTypeLiteral',
          members: tsBody,
        };
      } else {
        /*
        spreads are a complicate thing for us to handle, sadly.
        in flow type spreads are modelled after object spreads; meaning that for
        { ...A, ...B } - all properties in B will explicitly replace any properties
        in A of the same name.
        ```
        type T1 = { a: string };
        type T2 = { a: number };
        type T3 = { ...T1, ...T2 };
        type A = T3['a'] // === number
        ```

        however in TS there are no object type spreads - you can only merge
        objects either via the intersection operator or via interface extends.

        For an interface extends - `interface B extends A { ... }` - TS enforces
        that the properties of B are all covariantly related to the same named
        properties in A.
        So we can't use an interface extends.

        For a type intersection - `type T = A & B;` - TS will (essentially) merge
        the types by intersecting each same named property in each type to calculate
        the resulting type. This has the effect of enforcing the same constraint
        as the interface case, however instead of an error it causes properties to
        become `never`:
        ```
        type T1 = { a: string };
        type T2 = { a: number };
        type T3 = T1 & T2;
        type A = T3['a'] // === string & number === never
        ```

        So in order for us to model flow's spreads we have to explicitly omit the keys
        from the proceeding type that might clash. We can do this pretty easily using
        TS's utility types:
        ```
        type T1 = { a: string };
        type T2 = { a: number };
        type T3 = Omit<T1, keyof T2> & T2;
        type A = T3['a'] // === number
        ```

        Unfortunately because we need to solve for the general case object type usage,
        it's going to be a bit ugly and complicated, sadly.

        If we had access to type information we would be able to skip some ugliness by
        checking to see if there is any overlap and skipping the omit step if there isn't.
        But alas - we're working purely syntactically.

        ```
        type T = { ...T1, b: string  };
        // becomes
        type T = Omit<T1, keyof { b: string }> & { b: string };
        ```
        ```
        type T = { ...T1, ...T2, ...T3, b: string  };
        // becomes
        type T =
          & Omit<T1, keyof (T2 | T3 | { b: string })>
          & Omit<T2, keyof (T3 | { b: string })>
          & Omit<T3, keyof { b: string }>
          & { b: string };
        ```

        Note that because it's going to be super ugly and complicated - for now we're going to disallow:
        - spreads in the middle
        - spreads at the end
        - spreads of things that aren't "Identifiers"
        */

        if (members.length > 0) {
          return unsupportedAnnotation(
            node,
            'object types with spreads, indexers and/or call properties at the same time',
          );
        }

        const typesToIntersect = [];
        for (const property of node.properties) {
          if (property.type === 'ObjectTypeSpreadProperty') {
            if (members.length > 0) {
              return unsupportedAnnotation(
                property,
                'object types with spreads in the middle or at the end',
              );
            }

            const spreadType = transformTypeAnnotationType(property.argument);
            if (spreadType.type !== 'TSTypeReference') {
              return unsupportedAnnotation(
                property,
                'object types with complex spreads',
              );
            }

            typesToIntersect.push(spreadType);
          } else if (property.type === 'ObjectTypeMappedTypeProperty') {
            // TODO - Support mapped types
            return unsupportedAnnotation(
              property,
              'object type with mapped type property',
            );
          } else {
            members.push({
              start: property.range[0],
              node: transform.ObjectTypeProperty(property),
            });
          }
        }

        const tsBody = members
          .sort((a, b) => a.start - b.start)
          .map(({node}) => node);
        const objectType = {
          type: 'TSTypeLiteral',
          members: tsBody,
        };

        const intersectionMembers: Array<TSESTree.TypeNode> = [];
        for (let i = 0; i < typesToIntersect.length; i += 1) {
          const currentType = typesToIntersect[i];
          const remainingTypes = typesToIntersect.slice(i + 1);
          intersectionMembers.push({
            type: 'TSTypeReference',
            typeName: {
              type: 'Identifier',
              name: 'Omit',
            },
            typeParameters: {
              type: 'TSTypeParameterInstantiation',
              params: [
                currentType,
                {
                  type: 'TSTypeOperator',
                  operator: 'keyof',
                  typeAnnotation: {
                    type: 'TSUnionType',
                    types: [...remainingTypes, objectType],
                  },
                },
              ],
            },
          });
        }
        intersectionMembers.push(objectType);

        return {
          type: 'TSIntersectionType',
          types: intersectionMembers,
        };
      }
    },
    ObjectTypeCallProperty(
      node: FlowESTree.ObjectTypeCallProperty,
    ): TSESTree.TSCallSignatureDeclaration {
      // the info is stored on the "value"
      const func = transform.FunctionTypeAnnotation(node.value);
      return {
        type: 'TSCallSignatureDeclaration',
        params: func.params,
        returnType: func.returnType,
        typeParameters: func.typeParameters,
      };
    },
    ObjectTypeIndexer(
      node: FlowESTree.ObjectTypeIndexer,
    ): TSESTree.TSIndexSignature {
      return {
        type: 'TSIndexSignature',
        parameters: [
          {
            type: 'Identifier',
            name: node.id == null ? '$$Key$$' : node.id.name,
            typeAnnotation: {
              type: 'TSTypeAnnotation',
              typeAnnotation: transformTypeAnnotationType(node.key),
            },
          },
        ],
        readonly: node.variance?.kind === 'plus',
        static: node.static,
        typeAnnotation: {
          type: 'TSTypeAnnotation',
          typeAnnotation: transformTypeAnnotationType(node.value),
        },
      };
    },
    ObjectTypeProperty(
      node: FlowESTree.ObjectTypeProperty,
    ): TSESTree.TSPropertySignature | TSESTree.TSMethodSignature {
      const key =
        node.key.type === 'Identifier'
          ? transform.Identifier(node.key)
          : transform.StringLiteral(node.key);

      if (node.method === true) {
        // flow has just one node for all object properties and relies upon the method flag
        // TS has separate nodes for methods and properties
        const func = transform.FunctionTypeAnnotation(node.value);
        return {
          type: 'TSMethodSignature',
          computed: false,
          key,
          kind: node.kind === 'init' ? 'method' : node.kind,
          optional: node.optional,
          params: func.params,
          returnType: func.returnType,
          static: node.static,
          typeParameters: func.typeParameters,
        };
      }

      if (node.kind === 'get' || node.kind === 'set') {
        // flow treats getters/setters as true property signatures (method === false)
        // TS treats them as method signatures
        const func = transform.FunctionTypeAnnotation(node.value);
        return {
          type: 'TSMethodSignature',
          computed: false,
          key,
          kind: node.kind,
          optional: false,
          params: func.params,
          // TS setters must not have a return type
          returnType: node.kind === 'set' ? undefined : func.returnType,
          static: node.static,
          // TS accessors cannot have type parameters
          typeParameters: undefined,
        };
      }

      return {
        type: 'TSPropertySignature',
        computed: false,
        key,
        optional: node.optional,
        readonly: node.variance?.kind === 'plus',
        static: node.static,
        typeAnnotation: {
          type: 'TSTypeAnnotation',
          typeAnnotation: transformTypeAnnotationType(node.value),
        },
      };
    },
    OpaqueType(node: FlowESTree.OpaqueType): TSESTree.TSTypeAliasDeclaration {
      return transform.DeclareOpaqueType(node);
    },
    OptionalIndexedAccessType(
      node: FlowESTree.OptionalIndexedAccessType,
    ): TSESTree.TSIndexedAccessType {
      // Foo?.[A][B]
      // ^^^^^^^^    optional = true
      // ^^^^^^^^^^^ optional = false
      if (node.optional === false) {
        return transform.IndexedAccessType(node);
      }

      // TS doesn't support optional index access so we have to wrap the object:
      // `T?.[K]` becomes `NonNullable<T>[K]`
      return {
        type: 'TSIndexedAccessType',
        objectType: {
          type: 'TSTypeReference',
          typeName: {
            type: 'Identifier',
            name: 'NonNullable',
          },
          typeParameters: {
            type: 'TSTypeParameterInstantiation',
            params: [transformTypeAnnotationType(node.objectType)],
          },
        },
        indexType: transformTypeAnnotationType(node.indexType),
      };
    },
    QualifiedTypeIdentifier(
      node: FlowESTree.QualifiedTypeIdentifier,
    ): TSESTree.TSQualifiedName {
      const qual = node.qualification;

      return {
        type: 'TSQualifiedName',
        left:
          qual.type === 'Identifier'
            ? transform.Identifier(qual, false)
            : transform.QualifiedTypeIdentifier(qual),
        right: transform.Identifier(node.id, false),
      };
    },
    QualifiedTypeofIdentifier(
      node: FlowESTree.QualifiedTypeofIdentifier,
    ): TSESTree.TSQualifiedName {
      const qual = node.qualification;

      return {
        type: 'TSQualifiedName',
        left:
          qual.type === 'Identifier'
            ? transform.Identifier(qual, false)
            : transform.QualifiedTypeofIdentifier(qual),
        right: transform.Identifier(node.id, false),
      };
    },
    RegExpLiteral(node: FlowESTree.RegExpLiteral): TSESTree.RegExpLiteral {
      return {
        type: 'Literal',
        raw: node.raw,
        regex: {
          pattern: node.regex.pattern,
          flags: node.regex.pattern,
        },
        value: node.value,
      };
    },
    StringLiteral(node: FlowESTree.StringLiteral): TSESTree.StringLiteral {
      return {
        type: 'Literal',
        raw: node.raw,
        value: node.value,
      };
    },
    StringLiteralTypeAnnotation(
      node: FlowESTree.StringLiteralTypeAnnotation,
    ): TSESTree.TSLiteralType {
      return {
        type: 'TSLiteralType',
        literal: ({
          type: 'Literal',
          value: node.value,
          raw: node.raw,
        }: TSESTree.StringLiteral),
      };
    },
    StringTypeAnnotation(
      _node: FlowESTree.StringTypeAnnotation,
    ): TSESTree.TSStringKeyword {
      return {
        type: 'TSStringKeyword',
      };
    },
    SymbolTypeAnnotation(
      _node: FlowESTree.SymbolTypeAnnotation,
    ): TSESTree.TSSymbolKeyword {
      return {
        type: 'TSSymbolKeyword',
      };
    },
    ThisTypeAnnotation(
      _node: FlowESTree.ThisTypeAnnotation,
    ): TSESTree.TSThisType {
      return {
        type: 'TSThisType',
      };
    },
    TupleTypeAnnotation(
      node: FlowESTree.TupleTypeAnnotation,
    ): TSESTree.TSTupleType | TSESTree.TSTypeOperator {
      const allReadOnly =
        node.types.length > 0 &&
        node.types.every(
          element =>
            element.type === 'TupleTypeLabeledElement' &&
            element.variance != null &&
            element.variance.kind === 'plus',
        );
      const tupleAnnot = {
        type: 'TSTupleType',
        elementTypes: node.types.map(element => {
          switch (element.type) {
            case 'TupleTypeLabeledElement':
              if (!allReadOnly && element.variance != null) {
                return unsupportedAnnotation(
                  element,
                  'tuple type element variance annotations',
                );
              }
              return {
                type: 'TSNamedTupleMember',
                label: transform.Identifier(element.label),
                optional: element.optional,
                elementType: transformTypeAnnotationType(element.elementType),
              };
            case 'TupleTypeSpreadElement': {
              const annot = transformTypeAnnotationType(element.typeAnnotation);
              return {
                type: 'TSRestType',
                typeAnnotation:
                  element.label != null
                    ? {
                        type: 'TSNamedTupleMember',
                        label: transform.Identifier(element.label),
                        optional: false,
                        elementType: annot,
                      }
                    : annot,
              };
            }
            default:
              return transformTypeAnnotationType(element);
          }
        }),
      };
      return allReadOnly
        ? {
            type: 'TSTypeOperator',
            operator: 'readonly',
            typeAnnotation: tupleAnnot,
          }
        : tupleAnnot;
    },
    TypeAlias(node: FlowESTree.TypeAlias): TSESTree.TSTypeAliasDeclaration {
      return transform.DeclareTypeAlias(node);
    },
    TypeAnnotation(node: FlowESTree.TypeAnnotation): TSESTree.TSTypeAnnotation {
      return {
        type: 'TSTypeAnnotation',
        typeAnnotation: transformTypeAnnotationType(node.typeAnnotation),
      };
    },
    TypeofTypeAnnotation(
      node: FlowESTree.TypeofTypeAnnotation,
    ): TSESTree.TSTypeQuery {
      switch (node.argument.type) {
        case 'Identifier':
          return {
            type: 'TSTypeQuery',
            exprName: transform.Identifier(node.argument),
            typeParameters: undefined,
          };
        case 'QualifiedTypeofIdentifier':
          return {
            type: 'TSTypeQuery',
            exprName: transform.QualifiedTypeofIdentifier(node.argument),
            typeParameters: undefined,
          };
      }
    },
    TypeParameter(node: FlowESTree.TypeParameter): TSESTree.TSTypeParameter {
      /*
      TODO - flow models variance as explicit syntax, but but TS resolves it automatically
      TS does have syntax for explicit variance, but you can introduce a TS error if the
      marked parameter isn't used in the location that TS expects them to be in.

      To make it easier for now let's just rely on TS's auto-resolution.

      ```
      const variance =
        new Set(
          node.variance == null
            ? // by default flow generics act invariantly
              ['in', 'out']
            : node.variance.kind === 'plus'
            ? // covariant
              ['out']
            : // contravariant
              ['in'],
        );
      ```
      */
      return {
        type: 'TSTypeParameter',
        name: {
          type: 'Identifier',
          name: node.name,
        },
        constraint:
          node.bound == null
            ? undefined
            : transformTypeAnnotationType(node.bound.typeAnnotation),
        default:
          node.default == null
            ? undefined
            : transformTypeAnnotationType(node.default),
        in: false,
        out: false,
        // in: variance.has('in'),
        // out: variance.has('out'),
      };
    },
    TypeParameterDeclaration(
      node: FlowESTree.TypeParameterDeclaration,
    ): TSESTree.TSTypeParameterDeclaration {
      return {
        type: 'TSTypeParameterDeclaration',
        params: node.params.map(transform.TypeParameter),
      };
    },
    TypeParameterInstantiation(
      node: FlowESTree.TypeParameterInstantiation,
    ): TSESTree.TSTypeParameterInstantiation {
      return {
        type: 'TSTypeParameterInstantiation',
        params: node.params.map(transformTypeAnnotationType),
      };
    },
    UnionTypeAnnotation(
      node: FlowESTree.UnionTypeAnnotation,
    ): TSESTree.TSUnionType {
      return {
        type: 'TSUnionType',
        types: node.types.map(transformTypeAnnotationType),
      };
    },
    VoidTypeAnnotation(
      _node: FlowESTree.VoidTypeAnnotation,
    ): TSESTree.TSVoidKeyword {
      return {
        type: 'TSVoidKeyword',
      };
    },
    ConditionalTypeAnnotation(
      node: FlowESTree.ConditionalTypeAnnotation,
    ): TSESTree.TSConditionalType {
      return {
        type: 'TSConditionalType',
        checkType: transformTypeAnnotationType(node.checkType),
        extendsType: transformTypeAnnotationType(node.extendsType),
        trueType: transformTypeAnnotationType(node.trueType),
        falseType: transformTypeAnnotationType(node.falseType),
      };
    },
    TypePredicateAnnotation(
      node: FlowESTree.TypePredicate,
    ): TSESTree.TSTypePredicate {
      return {
        type: 'TSTypePredicate',
        asserts: node.asserts,
        parameterName: transform.Identifier(node.parameterName, false),
        typeAnnotation: node.typeAnnotation && {
          type: 'TSTypeAnnotation',
          typeAnnotation: transformTypeAnnotationType(node.typeAnnotation),
        },
      };
    },
    InferTypeAnnotation(
      node: FlowESTree.InferTypeAnnotation,
    ): TSESTree.TSInferType {
      return {
        type: 'TSInferType',
        typeParameter: transform.TypeParameter(node.typeParameter),
      };
    },
    KeyofTypeAnnotation(
      node: FlowESTree.KeyofTypeAnnotation,
    ): TSESTree.TSTypeOperator {
      return {
        type: 'TSTypeOperator',
        operator: 'keyof',
        typeAnnotation: transformTypeAnnotationType(node.argument),
      };
    },
  };

  // wrap each transform so that we automatically preserve jsdoc comments
  // this just saves us manually wiring up every single case
  for (const key of Object.keys(transform)) {
    const originalFn: $FlowFixMe = transform[key];
    // $FlowExpectedError[cannot-write]
    // $FlowExpectedError[missing-local-annot]
    transform[key] = (node, ...args) => {
      const result = originalFn(node, ...args);
      if (Array.isArray(result)) {
        cloneJSDocCommentsToNewNode(node, result[0]);
      } else {
        cloneJSDocCommentsToNewNode(node, result);
      }
      return result;
    };
  }

  return [transform, code];
};
