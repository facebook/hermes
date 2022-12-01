/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {TypeAnnotationType} from 'hermes-estree';

import fs from 'fs';
import path from 'path';
import {GetHermesESTreeJSON} from '../scripts/utils/scriptUtils';
import {parseForESLint} from 'hermes-eslint';
import {traverse} from 'hermes-transform';

type PropertyEntry = $ReadOnly<{
  name: string,
  optional: boolean,
  value: TypeAnnotationType,
}>;
type InterfaceEntry = $ReadOnly<{
  interface: $ReadOnly<{
    name: string,
    extends: $ReadOnlyArray<string>,
  }>,
  properties: Map<string, PropertyEntry>,
}>;

function getInterfaces(): $ReadOnlyMap<
  string,
  $ReadOnly<{
    ...InterfaceEntry,
    properties: $ReadOnlyMap<string, PropertyEntry>,
  }>,
> {
  const code = fs.readFileSync(
    path.resolve(__dirname, '..', 'hermes-estree', 'src', 'types.js'),
    'utf8',
  );
  const {ast, scopeManager} = parseForESLint(code);

  const interfaces = new Map<string, InterfaceEntry>();
  traverse(code, ast, scopeManager, () => {
    return {
      InterfaceDeclaration(node) {
        // the granular flow-types are a generated artefact that we don't have access to
        // so this check is to make flow understand the relationship
        if (node.type !== 'InterfaceDeclaration') {
          return;
        }

        const properties = new Map();

        // add the properties directly declared on the node
        for (const property of node.body.properties) {
          if (property.type === 'ObjectTypeSpreadProperty') {
            continue;
          }
          properties.set(property.key.name, {
            name: property.key.name,
            optional: property.optional,
            value: property.value,
          });
        }

        interfaces.set(node.id.name, {
          interface: {
            name: node.id.name,
            extends: node.extends.map(e => e.id.name),
          },
          properties,
        });
      },
    };
  });

  // add the properties declared on extended interfaces
  for (const {interface: iface, properties} of interfaces.values()) {
    for (const extendedName of iface.extends) {
      const extendedInterface = interfaces.get(extendedName);
      if (extendedInterface == null) {
        // flow will error on the types file here - but fail the test just in case
        throw new Error(
          `Unable to find the extended interface ${extendedName}.`,
        );
      }

      for (const [name, property] of extendedInterface.properties.entries()) {
        if (!properties.has(name)) {
          properties.set(name, property);
        }
      }
    }
  }

  return interfaces;
}

const typesThatShouldBeSkipped = new Set([
  // These types have a special union type declared to allow consumers to refine on `.computed`
  'BinaryExpression',
  'DeclareExportDeclaration',
  'ExportNamedDeclaration',
  'MemberExpression',
  'MethodDefinition',
  'ObjectTypeProperty',
  'Property',
  'PropertyDefinition',
]);
const propertiesThatShouldBeSkipped = new Map([
  [
    'TemplateElement',
    new Set([
      // hermes declares these directly on the node - but ESTree declares them on
      // a nested object under the `value` property.
      'cooked',
      'raw',
    ]),
  ],
  [
    'RegExpLiteral',
    new Set([
      // hermes declares these directly on the node - but ESTree declares them on
      // a nested object under the `regex` property.
      'pattern',
      'flags',
    ]),
  ],
]);
const propertiesWithIncorrectOptionalFlagInHermes = new Map([
  [
    'ImportDeclaration',
    new Set([
      // hermes has this marked as optional, but it always returns an array
      // https://www.internalfb.com/code/fbsource/[03317c92fabb32841bec7d90d297595d7392f6ea]/xplat/hermes/lib/Parser/JSParserImpl.cpp?lines=5846%2C5859-5860
      'assertions',
    ]),
  ],
  [
    'ImportSpecifier',
    new Set([
      // hermes has this marked as non-optional, but it will emit `null` instead of `'value'` for import
      // specifiers with no explicit kind token
      'importKind',
    ]),
  ],
  [
    'InterfaceTypeAnnotation',
    new Set([
      // hermes has this marked as optional, but it will throw if the body is missing
      // https://www.internalfb.com/code/fbsource/[03317c92fabb32841bec7d90d297595d7392f6ea]/xplat/hermes/lib/Parser/JSParserImpl-flow.cpp?lines=268-269
      'body',
    ]),
  ],
  [
    'YieldExpression',
    new Set([
      // hermes has this marked as optional, but it always emits a boolean
      // https://www.internalfb.com/code/fbsource/[cf84f54840c93b8b2c5dbefb137bc8925c636a65]/xplat/hermes/lib/Parser/JSParserImpl.cpp?lines=4203
      'delegate',
    ]),
  ],
]);

describe('All nodes declared by hermes should have an interface in hermes-estree that is of the correct shape.', () => {
  const interfaces = getInterfaces();

  for (const node of GetHermesESTreeJSON()) {
    if (typesThatShouldBeSkipped.has(node.name)) {
      describe.skip(node.name, () => {
        it.skip('was skipped due to being included in `typesThatShouldBeSkipped`', () => {});
      });
      continue;
    }

    describe(node.name, () => {
      const iface = interfaces.get(node.name);
      it('has an interface declared', () => {
        expect(iface).not.toBeUndefined();
      });

      if (!iface) {
        // the expect asserts this holds true
        return;
      }

      for (const {name, optional} of node.arguments) {
        // property is known to be incorrect
        if (propertiesThatShouldBeSkipped.get(node.name)?.has(name)) {
          it.skip('was marked to be skipped in `propertiesThatShouldBeSkipped`', () => {});
          continue;
        }

        describe(`property - ${name}`, () => {
          const property = iface.properties.get(name);
          it('is defined on the interface', () => {
            expect(property).not.toBeUndefined();
          });

          if (!property) {
            // the expect asserts this holds true
            return;
          }

          // property is known to have incorrect "optional"
          if (
            propertiesWithIncorrectOptionalFlagInHermes
              .get(node.name)
              ?.has(name)
          ) {
            it.skip('has an incorrect optional flag in the hermes spec so we skip this test', () => {});
            return;
          }

          const propertyHasOptionalToken = property.optional;
          let propertyHasNullType = (() => {
            if (property.value.type === 'NullLiteralTypeAnnotation') {
              return true;
            }
            if (property.value.type === 'UnionTypeAnnotation') {
              return property.value.types.some(
                t => t.type === 'NullLiteralTypeAnnotation',
              );
            }
            return false;
          })();

          const propertyIsOptional =
            propertyHasOptionalToken || propertyHasNullType;
          if (optional) {
            it('should be marked as optional', () => {
              expect(propertyIsOptional).toBe(optional);
            });
          } else {
            it('should not be marked as optional', () => {
              expect(propertyIsOptional).toBe(optional);
            });
          }
        });
      }
    });
  }
});
