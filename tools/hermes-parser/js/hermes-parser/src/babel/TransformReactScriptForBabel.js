/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

/**
 * This transform strips Flow types that are not supported past Babel 7.
 *
 * It is expected that all transforms create valid ESTree AST output. If
 * the transform requires outputting Babel specific AST nodes then it
 * should live in `ConvertESTreeToBabel.js`
 */

'use strict';

import type {ParserOptions} from '../ParserOptions';
import type {
  Program,
  ESNode,
  DeclareComponent,
  DeclareVariable,
  ComponentDeclaration,
  FunctionDeclaration,
  TypeAnnotation,
  ComponentParameter,
  SourceLocation,
  Position,
  ObjectPattern,
  Identifier,
  Range,
  RestElement,
  DestructuringObjectProperty,
} from 'hermes-estree';

import {SimpleTransform} from '../transform/SimpleTransform';

const nodeWith = SimpleTransform.nodeWith;

// Rely on the mapper to fix up parent relationships.
const EMPTY_PARENT: $FlowFixMe = null;

function createSyntaxError(node: ESNode, err: string): SyntaxError {
  const syntaxError = new SyntaxError(err);
  // $FlowExpectedError[prop-missing]
  syntaxError.loc = {
    line: node.loc.start.line,
    column: node.loc.start.column,
  };

  return syntaxError;
}

function createDefaultPosition(): Position {
  return {
    line: 1,
    column: 0,
  };
}

function mapDeclareComponent(node: DeclareComponent): DeclareVariable {
  return {
    type: 'DeclareVariable',
    id: nodeWith(node.id, {
      typeAnnotation: {
        type: 'TypeAnnotation',
        typeAnnotation: {
          type: 'AnyTypeAnnotation',
          loc: node.loc,
          range: node.range,
          parent: EMPTY_PARENT,
        },
        loc: node.loc,
        range: node.range,
        parent: EMPTY_PARENT,
      },
    }),
    kind: 'const',
    loc: node.loc,
    range: node.range,
    parent: node.parent,
  };
}

function getComponentParameterName(
  paramName: ComponentParameter['name'],
): string {
  switch (paramName.type) {
    case 'Identifier':
      return paramName.name;
    case 'Literal':
      return paramName.value;
    default:
      throw createSyntaxError(
        paramName,
        `Unknown Component parameter name type of "${paramName.type}"`,
      );
  }
}

function createPropsTypeAnnotation(
  loc: ?SourceLocation,
  range: ?Range,
): TypeAnnotation {
  // Create empty loc for type annotation nodes
  const createParamsTypeLoc = () => ({
    loc: {
      start: loc?.start != null ? loc.start : createDefaultPosition(),
      end: loc?.end != null ? loc.end : createDefaultPosition(),
    },
    range: range ?? [0, 0],
    parent: EMPTY_PARENT,
  });

  return {
    type: 'TypeAnnotation',
    typeAnnotation: {
      type: 'GenericTypeAnnotation',
      id: {
        type: 'Identifier',
        name: '$ReadOnly',
        optional: false,
        typeAnnotation: null,
        ...createParamsTypeLoc(),
      },
      typeParameters: {
        type: 'TypeParameterInstantiation',
        params: [
          {
            type: 'ObjectTypeAnnotation',
            callProperties: [],
            properties: [],
            indexers: [],
            internalSlots: [],
            exact: false,
            inexact: true,
            ...createParamsTypeLoc(),
          },
        ],
        ...createParamsTypeLoc(),
      },
      ...createParamsTypeLoc(),
    },
    ...createParamsTypeLoc(),
  };
}

function mapComponentParameters(
  params: $ReadOnlyArray<ComponentParameter | RestElement>,
): $ReadOnlyArray<ObjectPattern | Identifier> {
  if (params.length === 0) {
    return [];
  }

  // Optimize `component Foo(...props: Props) {}` to `function Foo(props: Props) {}
  if (
    params.length === 1 &&
    params[0].type === 'RestElement' &&
    params[0].argument.type === 'Identifier'
  ) {
    const restElementArgument = params[0].argument;
    return [
      nodeWith(restElementArgument, {
        typeAnnotation: createPropsTypeAnnotation(
          restElementArgument.typeAnnotation?.loc,
          restElementArgument.typeAnnotation?.range,
        ),
      }),
    ];
  }

  const properties = params.map(mapComponentParameter);

  const lastProperty = properties[properties.length - 1];

  return [
    {
      type: 'ObjectPattern',
      properties,
      typeAnnotation: createPropsTypeAnnotation(
        {
          start: lastProperty.loc.end,
          end: lastProperty.loc.end,
        },
        [lastProperty.range[1], lastProperty.range[1]],
      ),
      loc: {
        start: properties[0].loc.start,
        end: lastProperty.loc.end,
      },
      range: [properties[0].range[0], lastProperty.range[1]],
      parent: EMPTY_PARENT,
    },
  ];
}

function mapComponentParameter(
  param: ComponentParameter | RestElement,
): DestructuringObjectProperty | RestElement {
  switch (param.type) {
    case 'RestElement': {
      const a = nodeWith(param, {
        typeAnnotation: null,
        argument: nodeWith(param.argument, {typeAnnotation: null}),
      });
      return a;
    }
    case 'ComponentParameter': {
      if (getComponentParameterName(param.name) === 'ref') {
        throw createSyntaxError(
          param,
          'Component parameters named "ref" are currently not supported',
        );
      }

      let value;
      if (param.local.type === 'AssignmentPattern') {
        value = nodeWith(param.local, {
          left: nodeWith(param.local.left, {
            typeAnnotation: null,
            optional: false,
          }),
        });
      } else {
        value = nodeWith(param.local, {
          typeAnnotation: null,
          optional: false,
        });
      }

      // Shorthand params
      if (
        param.name.type === 'Identifier' &&
        param.shorthand &&
        (value.type === 'Identifier' || value.type === 'AssignmentPattern')
      ) {
        return {
          type: 'Property',
          key: param.name,
          kind: 'init',
          value,
          method: false,
          shorthand: true,
          computed: false,
          loc: param.loc,
          range: param.range,
          parent: EMPTY_PARENT,
        };
      }

      // Complex params
      return {
        type: 'Property',
        key: param.name,
        kind: 'init',
        value,
        method: false,
        shorthand: false,
        computed: false,
        loc: param.loc,
        range: param.range,
        parent: EMPTY_PARENT,
      };
    }
    default: {
      throw createSyntaxError(
        param,
        `Unknown Component parameter type of "${param.type}"`,
      );
    }
  }
}

function mapComponentDeclaration(
  node: ComponentDeclaration,
): FunctionDeclaration {
  let rendersType = node.rendersType;
  if (rendersType == null) {
    // Create empty loc for return type annotation nodes
    const createRendersTypeLoc = () => ({
      loc: {
        start: node.body.loc.end,
        end: node.body.loc.end,
      },
      range: [node.body.range[1], node.body.range[1]],
      parent: EMPTY_PARENT,
    });

    rendersType = {
      type: 'TypeAnnotation',
      typeAnnotation: {
        type: 'GenericTypeAnnotation',
        id: {
          type: 'QualifiedTypeIdentifier',
          qualification: {
            type: 'Identifier',
            name: 'React',
            optional: false,
            typeAnnotation: null,
            ...createRendersTypeLoc(),
          },
          id: {
            type: 'Identifier',
            name: 'Node',
            optional: false,
            typeAnnotation: null,
            ...createRendersTypeLoc(),
          },
          ...createRendersTypeLoc(),
        },
        typeParameters: null,
        ...createRendersTypeLoc(),
      },
      ...createRendersTypeLoc(),
    };
  }

  const params = mapComponentParameters(node.params);

  return {
    type: 'FunctionDeclaration',
    id: node.id,
    __componentDeclaration: true,
    typeParameters: node.typeParameters,
    params,
    returnType: rendersType,
    body: node.body,
    async: false,
    generator: false,
    predicate: null,
    loc: node.loc,
    range: node.range,
    parent: node.parent,
  };
}

export function transformProgram(
  program: Program,
  _options: ParserOptions,
): Program {
  return SimpleTransform.transformProgram(program, {
    transform(node: ESNode) {
      switch (node.type) {
        case 'DeclareComponent': {
          return mapDeclareComponent(node);
        }
        case 'ComponentDeclaration': {
          return mapComponentDeclaration(node);
        }
        default: {
          return node;
        }
      }
    },
  });
}
