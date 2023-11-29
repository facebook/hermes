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
  VariableDeclaration,
  ModuleDeclaration,
  Statement,
  AssignmentPattern,
  BindingName,
} from 'hermes-estree';

import {SimpleTransform} from '../transform/SimpleTransform';
import {shallowCloneNode} from '../transform/astNodeMutationHelpers';
import {SimpleTraverser} from '../traverse/SimpleTraverser';
import {createSyntaxError} from '../utils/createSyntaxError';

const nodeWith = SimpleTransform.nodeWith;

// Rely on the mapper to fix up parent relationships.
const EMPTY_PARENT: $FlowFixMe = null;

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
): $ReadOnly<{
  props: ?(ObjectPattern | Identifier),
  ref: ?(BindingName | AssignmentPattern),
}> {
  if (params.length === 0) {
    return {props: null, ref: null};
  }

  // Optimize `component Foo(...props: Props) {}` to `function Foo(props: Props) {}
  if (
    params.length === 1 &&
    params[0].type === 'RestElement' &&
    params[0].argument.type === 'Identifier'
  ) {
    const restElementArgument = params[0].argument;
    return {
      props: nodeWith(restElementArgument, {
        typeAnnotation: createPropsTypeAnnotation(
          restElementArgument.typeAnnotation?.loc,
          restElementArgument.typeAnnotation?.range,
        ),
      }),
      ref: null,
    };
  }

  // Filter out any ref param and capture it's details.
  let refParam = null;
  const paramsWithoutRef = params.filter(param => {
    if (
      param.type === 'ComponentParameter' &&
      getComponentParameterName(param.name) === 'ref'
    ) {
      refParam = param;
      return false;
    }

    return true;
  });

  const propsProperties = paramsWithoutRef.flatMap(mapComponentParameter);

  let props = null;
  if (propsProperties.length === 0) {
    if (refParam == null) {
      throw new Error(
        'TransformReactScriptForBabel: Invalid state, ref should always be set at this point if props are empty',
      );
    }
    const emptyParamsLoc = {
      start: refParam.loc.start,
      end: refParam.loc.start,
    };
    const emptyParamsRange = [refParam.range[0], refParam.range[0]];
    // no properties provided (must have had a single ref)
    props = {
      type: 'Identifier',
      name: '_$$empty_props_placeholder$$',
      optional: false,
      typeAnnotation: createPropsTypeAnnotation(
        emptyParamsLoc,
        emptyParamsRange,
      ),
      loc: emptyParamsLoc,
      range: emptyParamsRange,
      parent: EMPTY_PARENT,
    };
  } else {
    const lastPropsProperty = propsProperties[propsProperties.length - 1];
    props = {
      type: 'ObjectPattern',
      properties: propsProperties,
      typeAnnotation: createPropsTypeAnnotation(
        {
          start: lastPropsProperty.loc.end,
          end: lastPropsProperty.loc.end,
        },
        [lastPropsProperty.range[1], lastPropsProperty.range[1]],
      ),
      loc: {
        start: propsProperties[0].loc.start,
        end: lastPropsProperty.loc.end,
      },
      range: [propsProperties[0].range[0], lastPropsProperty.range[1]],
      parent: EMPTY_PARENT,
    };
  }

  let ref = null;
  if (refParam != null) {
    ref = refParam.local;
  }

  return {
    props,
    ref,
  };
}

function mapComponentParameter(
  param: ComponentParameter | RestElement,
): Array<DestructuringObjectProperty | RestElement> {
  switch (param.type) {
    case 'RestElement': {
      switch (param.argument.type) {
        case 'Identifier': {
          const a = nodeWith(param, {
            typeAnnotation: null,
            argument: nodeWith(param.argument, {typeAnnotation: null}),
          });
          return [a];
        }
        case 'ObjectPattern': {
          return param.argument.properties.map(property => {
            return nodeWith(property, {
              typeAnnotation: null,
            });
          });
        }
        default: {
          throw createSyntaxError(
            param,
            `Unhandled ${param.argument.type} encountered in restParameter`,
          );
        }
      }
    }
    case 'ComponentParameter': {
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
        return [
          {
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
          },
        ];
      }

      // Complex params
      return [
        {
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
        },
      ];
    }
    default: {
      throw createSyntaxError(
        param,
        `Unknown Component parameter type of "${param.type}"`,
      );
    }
  }
}

type ForwardRefDetails = {
  forwardRefStatement: VariableDeclaration,
  internalCompId: Identifier,
  forwardRefCompId: Identifier,
};

function createForwardRefWrapper(
  originalComponent: ComponentDeclaration,
): ForwardRefDetails {
  const internalCompId = {
    type: 'Identifier',
    name: `${originalComponent.id.name}_withRef`,
    optional: false,
    typeAnnotation: null,
    loc: originalComponent.id.loc,
    range: originalComponent.id.range,
    parent: EMPTY_PARENT,
  };
  return {
    forwardRefStatement: {
      type: 'VariableDeclaration',
      kind: 'const',
      declarations: [
        {
          type: 'VariableDeclarator',
          id: shallowCloneNode(originalComponent.id),
          init: {
            type: 'CallExpression',
            callee: {
              type: 'MemberExpression',
              object: {
                type: 'Identifier',
                name: 'React',
                optional: false,
                typeAnnotation: null,
                loc: originalComponent.loc,
                range: originalComponent.range,
                parent: EMPTY_PARENT,
              },
              property: {
                type: 'Identifier',
                name: 'forwardRef',
                optional: false,
                typeAnnotation: null,
                loc: originalComponent.loc,
                range: originalComponent.range,
                parent: EMPTY_PARENT,
              },
              computed: false,
              optional: false,
              loc: originalComponent.loc,
              range: originalComponent.range,
              parent: EMPTY_PARENT,
            },
            arguments: [shallowCloneNode(internalCompId)],
            typeArguments: null,
            optional: false,
            loc: originalComponent.loc,
            range: originalComponent.range,
            parent: EMPTY_PARENT,
          },
          loc: originalComponent.loc,
          range: originalComponent.range,
          parent: EMPTY_PARENT,
        },
      ],
      loc: originalComponent.loc,
      range: originalComponent.range,
      parent: originalComponent.parent,
    },
    internalCompId: internalCompId,
    forwardRefCompId: originalComponent.id,
  };
}

function mapComponentDeclaration(node: ComponentDeclaration): {
  comp: FunctionDeclaration,
  forwardRefDetails: ?ForwardRefDetails,
} {
  // Create empty loc for return type annotation nodes
  const createRendersTypeLoc = () => ({
    loc: {
      start: node.body.loc.end,
      end: node.body.loc.end,
    },
    range: [node.body.range[1], node.body.range[1]],
    parent: EMPTY_PARENT,
  });
  const returnType: TypeAnnotation = {
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

  const {props, ref} = mapComponentParameters(node.params);

  let forwardRefDetails: ?ForwardRefDetails = null;

  if (ref != null) {
    forwardRefDetails = createForwardRefWrapper(node);
  }

  const comp = {
    type: 'FunctionDeclaration',
    id:
      forwardRefDetails != null
        ? shallowCloneNode(forwardRefDetails.internalCompId)
        : shallowCloneNode(node.id),
    __componentDeclaration: true,
    typeParameters: node.typeParameters,
    params: props == null ? [] : ref == null ? [props] : [props, ref],
    returnType,
    body: node.body,
    async: false,
    generator: false,
    predicate: null,
    loc: node.loc,
    range: node.range,
    parent: node.parent,
  };

  return {comp, forwardRefDetails};
}

/**
 * Scan a list of statements and return the position of the
 * first statement that contains a reference to a given component
 * or null of no references were found.
 */
function scanForFirstComponentReference(
  compName: string,
  bodyList: Array<Statement | ModuleDeclaration>,
): ?number {
  for (let i = 0; i < bodyList.length; i++) {
    const bodyNode = bodyList[i];
    let referencePos = null;
    SimpleTraverser.traverse(bodyNode, {
      enter(node: ESNode) {
        switch (node.type) {
          case 'Identifier': {
            if (node.name === compName) {
              // We found a reference, record it and stop.
              referencePos = i;
              throw SimpleTraverser.Break;
            }
          }
        }
      },
      leave(_node: ESNode) {},
    });
    if (referencePos != null) {
      return referencePos;
    }
  }

  return null;
}

function mapComponentDeclarationIntoList(
  node: ComponentDeclaration,
  newBody: Array<Statement | ModuleDeclaration>,
  insertExport?: (Identifier | FunctionDeclaration) => ModuleDeclaration,
) {
  const {comp, forwardRefDetails} = mapComponentDeclaration(node);
  if (forwardRefDetails != null) {
    // Scan for references to our component.
    const referencePos = scanForFirstComponentReference(
      forwardRefDetails.forwardRefCompId.name,
      newBody,
    );

    // If a reference is found insert the forwardRef statement before it (to simulate function hoisting).
    if (referencePos != null) {
      newBody.splice(referencePos, 0, forwardRefDetails.forwardRefStatement);
    } else {
      newBody.push(forwardRefDetails.forwardRefStatement);
    }

    newBody.push(comp);

    if (insertExport != null) {
      newBody.push(insertExport(forwardRefDetails.forwardRefCompId));
    }
    return;
  }

  newBody.push(insertExport != null ? insertExport(comp) : comp);
}

function mapStatementList(
  stmts: $ReadOnlyArray<Statement | ModuleDeclaration>,
) {
  const newBody: Array<Statement | ModuleDeclaration> = [];
  for (const node of stmts) {
    switch (node.type) {
      case 'ComponentDeclaration': {
        mapComponentDeclarationIntoList(node, newBody);
        break;
      }
      case 'ExportNamedDeclaration': {
        if (node.declaration?.type === 'ComponentDeclaration') {
          mapComponentDeclarationIntoList(
            node.declaration,
            newBody,
            componentOrRef => {
              switch (componentOrRef.type) {
                case 'FunctionDeclaration': {
                  // No ref, so we can export the component directly.
                  return nodeWith(node, {declaration: componentOrRef});
                }
                case 'Identifier': {
                  // If a ref is inserted, we should just export that id.
                  return {
                    type: 'ExportNamedDeclaration',
                    declaration: null,
                    specifiers: [
                      {
                        type: 'ExportSpecifier',
                        exported: shallowCloneNode(componentOrRef),
                        local: shallowCloneNode(componentOrRef),
                        loc: node.loc,
                        range: node.range,
                        parent: EMPTY_PARENT,
                      },
                    ],
                    exportKind: 'value',
                    source: null,
                    loc: node.loc,
                    range: node.range,
                    parent: node.parent,
                  };
                }
              }
            },
          );
          break;
        }

        newBody.push(node);
        break;
      }
      case 'ExportDefaultDeclaration': {
        if (node.declaration?.type === 'ComponentDeclaration') {
          mapComponentDeclarationIntoList(
            node.declaration,
            newBody,
            componentOrRef => nodeWith(node, {declaration: componentOrRef}),
          );
          break;
        }

        newBody.push(node);
        break;
      }
      default: {
        newBody.push(node);
      }
    }
  }

  return newBody;
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
        case 'Program':
        case 'BlockStatement': {
          return nodeWith(node, {body: mapStatementList(node.body)});
        }
        case 'SwitchCase': {
          return nodeWith(node, {
            /* $FlowExpectedError[incompatible-call] We know `mapStatementList` will
               not return `ModuleDeclaration` nodes if it is not passed any */
            consequent: mapStatementList(node.consequent),
          });
        }
        case 'ComponentDeclaration': {
          throw createSyntaxError(
            node,
            `Components must be defined at the top level of a module or within a ` +
              `BlockStatement, instead got parent of "${node.parent?.type}".`,
          );
        }
        default: {
          return node;
        }
      }
    },
  });
}
