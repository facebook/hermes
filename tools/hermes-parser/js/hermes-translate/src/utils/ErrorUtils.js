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

import type {ESNode, TypeAnnotationType} from 'hermes-estree';
import type {TranslationContext} from './TranslationUtils';
import type {DetachedNode} from 'hermes-transform';

import {t} from 'hermes-transform';
import {codeFrameColumns} from '@babel/code-frame';

export function flowFixMeOrError(
  container: ESNode,
  message: string,
  context: TranslationContext,
): DetachedNode<TypeAnnotationType> {
  if (context.recoverFromErrors) {
    return t.GenericTypeAnnotation({id: t.Identifier({name: '$FlowFixMe'})});
  }
  throw translationError(container, message, context);
}

export function translationError(
  node: ESNode,
  message: string,
  context: TranslationContext,
): Error {
  return new Error(buildCodeFrame(node, context.code, message));
}

function buildCodeFrame(node: ESNode, code: string, message: string): string {
  // babel uses 1-indexed columns
  const locForBabel = {
    start: {
      line: node.loc.start.line,
      column: node.loc.start.column + 1,
    },
    end: {
      line: node.loc.end.line,
      column: node.loc.end.column + 1,
    },
  };
  return codeFrameColumns(code, locForBabel, {
    linesAbove: 0,
    linesBelow: 0,
    highlightCode: process.env.NODE_ENV !== 'test',
    message: message,
  });
}
