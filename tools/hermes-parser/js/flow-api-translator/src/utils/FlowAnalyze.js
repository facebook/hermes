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

import type {AFunction, TypeAnnotation, ESNode} from 'hermes-estree';
import type {TranslationContext, Dep} from './TranslationUtils';

import {t} from 'hermes-transform';
import {SimpleTraverser} from 'hermes-parser';

export function analyzeFunctionReturn(func: AFunction): TypeAnnotation {
  const returnType = func.returnType;
  if (returnType != null) {
    return returnType;
  }

  // We trust Flow has validated this function to only return void
  // $FlowFixMe[incompatible-return]
  return t.TypeAnnotation({typeAnnotation: t.VoidTypeAnnotation()});
}

export function analyzeTypeDependencies(
  rootNode: ESNode,
  context: TranslationContext,
): $ReadOnlyArray<Dep> {
  const deps = [];
  SimpleTraverser.traverse(rootNode, {
    enter(node: ESNode) {
      if (node.type === 'Identifier' || node.type === 'JSXIdentifier') {
        const variable = context.referenceMap.get(node);
        if (variable != null) {
          deps.push(variable.name);
        }
      }
    },
    leave() {},
  });
  return deps;
}
