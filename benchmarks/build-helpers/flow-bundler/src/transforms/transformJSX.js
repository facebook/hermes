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

import type {Variable, ScopeManager, Scope} from 'hermes-eslint';
import type {
  ESNode,
  Identifier,
  JSXIdentifier,
  ModuleDeclaration,
  Statement,
  Program,
  ArrayExpression,
  Expression,
  JSXAttribute,
  JSXChild,
  JSXElement,
  JSXOpeningElement,
  ObjectExpression,
  StringLiteral,
} from 'hermes-estree';
import type {ParseResult} from '../utils';
import type {MaybeDetachedNode} from 'hermes-transform/dist/detachedNode';

import {transformAST} from 'hermes-transform/dist/transform/transformAST';
import {t} from 'hermes-transform';

const REACT_JSX_NAME = 'React$jsx';
const REACT_FRAGMENT_NAME = 'React$Fragment';

export default async function transformJSX(
  parseResult: ParseResult,
): Promise<{ast: Program, code: string}> {
  const {ast, astWasMutated, mutatedCode} = transformAST(
    parseResult,
    context => {
      function convertOpeningElementToJSXName(
        openingElement: JSXOpeningElement,
      ): MaybeDetachedNode<Identifier | StringLiteral> {
        switch (openingElement.name.type) {
          case 'JSXIdentifier': {
            const name = openingElement.name.name;
            if (
              name[0] === name[0].toLowerCase() &&
              // This is needed because of Flow Bundler runs first and renames
              // components (so the name could be lower case).
              !name.includes('$')
            ) {
              return t.StringLiteral({
                value: name,
              });
            }
            return t.Identifier({name});
          }
          default: {
            throw new Error(
              context.buildCodeFrame(
                openingElement,
                `Unknown JSX element type "${openingElement.name.type}"`,
              ),
            );
          }
        }
      }

      function convertJSXAttributeValueToExpression(
        jsxAttributeValue: JSXAttribute['value'],
      ): MaybeDetachedNode<Expression> {
        if (jsxAttributeValue == null) {
          return t.BooleanLiteral({value: true});
        }

        switch (jsxAttributeValue.type) {
          case 'JSXExpressionContainer': {
            if (jsxAttributeValue.expression.type === 'JSXEmptyExpression') {
              return t.BooleanLiteral({value: true});
            }

            return jsxAttributeValue.expression;
          }
          case 'StringLiteral': {
            return t.StringLiteral({value: jsxAttributeValue.value});
          }
          case 'JSXEmptyExpression': {
            return t.BooleanLiteral({value: true});
          }
          default: {
            throw new Error(
              context.buildCodeFrame(
                jsxAttributeValue,
                `Unknown JSX attribute value "${jsxAttributeValue.type}"`,
              ),
            );
          }
        }
      }

      function convertJSXElementToProps(
        jsxElement: JSXElement,
      ): [MaybeDetachedNode<ObjectExpression>, MaybeDetachedNode<Expression>] {
        let key: MaybeDetachedNode<Expression> = t.NullLiteral();

        const attributes = jsxElement.openingElement.attributes
          .map(attribute => {
            switch (attribute.type) {
              case 'JSXAttribute': {
                if (attribute.name.name === 'key') {
                  key = convertJSXAttributeValueToExpression(attribute.value);
                  return null;
                }

                return t.ObjectProperty({
                  key: t.Identifier({name: attribute.name.name}),
                  value: convertJSXAttributeValueToExpression(attribute.value),
                  computed: false,
                  method: false,
                  shorthand: false,
                  kind: 'init',
                });
              }
              case 'JSXSpreadAttribute': {
                return t.SpreadElement({argument: attribute.argument});
              }
              default: {
                throw new Error(
                  context.buildCodeFrame(
                    attribute,
                    `Unknown JSX attribute "${attribute.type}"`,
                  ),
                );
              }
            }
          })
          .filter(Boolean);

        if (jsxElement.children.length > 0) {
          attributes.push(
            t.ObjectProperty({
              key: t.Identifier({name: 'children'}),
              value: convertJSXChildren(jsxElement.children),
              computed: false,
              method: false,
              shorthand: false,
              kind: 'init',
            }),
          );
        }

        return [
          t.ObjectExpression({
            properties: attributes,
          }),
          key,
        ];
      }

      function convertJSXChildren(
        children: $ReadOnlyArray<JSXChild>,
      ): MaybeDetachedNode<ArrayExpression | Expression> {
        const elements = children.map(convertJSXChild).filter(Boolean);
        if (elements.length === 1) {
          return elements[0];
        }
        return t.ArrayExpression({
          elements,
          trailingComma: false,
        });
      }

      function convertJSXChild(
        child: JSXChild,
      ): ?MaybeDetachedNode<Expression> {
        switch (child.type) {
          case 'JSXElement': {
            const [props, key] = convertJSXElementToProps(child);
            return t.CallExpression({
              callee: t.Identifier({name: REACT_JSX_NAME}),
              arguments: [
                convertOpeningElementToJSXName(child.openingElement),
                props,
                key,
              ],
            });
          }
          case 'JSXFragment': {
            return t.CallExpression({
              callee: t.Identifier({name: REACT_JSX_NAME}),
              arguments: [
                t.Identifier({name: REACT_FRAGMENT_NAME}),
                t.ObjectExpression({
                  properties: [
                    t.ObjectProperty({
                      key: t.Identifier({name: 'children'}),
                      value: convertJSXChildren(child.children),
                      computed: false,
                      method: false,
                      shorthand: false,
                      kind: 'init',
                    }),
                  ],
                }),
                t.NullLiteral(),
              ],
            });
          }
          case 'JSXText': {
            const lines = child.value.split(/\r\n|\n|\r/);
            let lastNonEmptyLine = 0;
            for (let i = 0; i < lines.length; i++) {
              if (lines[i].match(/[^ \t]/)) {
                lastNonEmptyLine = i;
              }
            }

            let str = '';
            for (let i = 0; i < lines.length; i++) {
              const line = lines[i];

              const isFirstLine = i === 0;
              const isLastLine = i === lines.length - 1;
              const isLastNonEmptyLine = i === lastNonEmptyLine;

              // replace rendered whitespace tabs with spaces
              let trimmedLine = line.replace(/\t/g, ' ');

              // trim whitespace touching a newline
              if (!isFirstLine) {
                trimmedLine = trimmedLine.replace(/^[ ]+/, '');
              }

              // trim whitespace touching an endline
              if (!isLastLine) {
                trimmedLine = trimmedLine.replace(/[ ]+$/, '');
              }

              if (trimmedLine) {
                if (!isLastNonEmptyLine) {
                  trimmedLine += ' ';
                }

                str += trimmedLine;
              }
            }

            if (str === '') {
              return null;
            }

            return t.StringLiteral({value: str});
          }
          case 'JSXEmptyExpression': {
            return t.NullLiteral();
          }
          case 'JSXExpressionContainer': {
            if (child.expression.type === 'JSXEmptyExpression') {
              return t.NullLiteral();
            }
            return child.expression;
          }
          default: {
            throw new Error(
              context.buildCodeFrame(
                child,
                `Unknown JSX child of type "${child.type}"`,
              ),
            );
          }
        }
      }

      return {
        JSXFragment(node) {
          context.skipTraversal();
          const resultNode = convertJSXChild(node);
          if (resultNode == null) {
            throw new Error('Expected non-empty JSX fragment');
          }
          context.replaceNode(node, resultNode);
        },
        JSXElement(node) {
          context.skipTraversal();
          const resultNode = convertJSXChild(node);
          if (resultNode == null) {
            throw new Error('Expected non-empty JSX element');
          }
          context.replaceNode(node, resultNode);
        },
      };
    },
  );

  return {ast, code: mutatedCode};
}
