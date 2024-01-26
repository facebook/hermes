/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ScopeManager} from 'hermes-eslint';
import type {Program, ESNode, BaseToken, Comment} from 'hermes-estree';
import type {BabelFile} from './fork/TransformESTreeToBabel';

import {parse} from 'hermes-transform';
import {promises as fs} from 'fs';
import * as path from 'path';
import {SimpleTraverser} from 'hermes-parser/dist/traverse/SimpleTraverser';
import * as StripComponentSyntax from 'hermes-parser/dist/estree/StripComponentSyntax';
import * as StripFlowTypesForBabel from 'hermes-parser/dist/estree/StripFlowTypesForBabel';
import * as TransformESTreeToBabel from './fork/TransformESTreeToBabel';

// $FlowExpectedError[cannot-resolve-module] Untyped third-party module
import {VISITOR_KEYS as babelVisitorKeys} from '@babel/types';

export type ParseResult = {
  ast: Program,
  scopeManager: ScopeManager,
  code: string,
};

export async function parseFile(filePath: string): Promise<ParseResult> {
  const fileContents = await fs.readFile(filePath, 'utf8');
  return parse(fileContents);
}

export async function writeFile(
  filePath: string,
  contents: string,
): Promise<void> {
  await fs.mkdir(path.dirname(filePath), {recursive: true});
  await fs.writeFile(filePath, contents);
}

interface BabelCommentBlock extends BaseToken {
  +type: 'CommentBlock';
  +value: string;
}
interface BabelCommentLine extends BaseToken {
  +type: 'CommentLine';
  +value: string;
}
type BabelComment = BabelCommentBlock | BabelCommentLine;

function hermesCommentToBabel(comment: Comment): BabelComment {
  switch (comment.type) {
    case 'Line': {
      return {
        type: 'CommentLine',
        value: comment.value,
        loc: comment.loc,
        range: comment.range,
      };
    }
    case 'Block': {
      return {
        type: 'CommentBlock',
        value: comment.value,
        loc: comment.loc,
        range: comment.range,
      };
    }
    default: {
      throw new Error(`Unknown comment type: ${comment.type}`);
    }
  }
}

export function hermesASTToBabel(ast: Program, file: string): BabelFile {
  const strippedAST = [
    StripComponentSyntax.transformProgram,
    StripFlowTypesForBabel.transformProgram,
  ].reduce((ast, transform) => transform(ast, {}), ast);

  SimpleTraverser.traverse(strippedAST, {
    enter(node: ESNode) {
      if (node.type === 'Program') {
        // Add docblock comment to first statement
        if (node.docblock != null) {
          const firstNode = node.body[0];
          if (firstNode != null) {
            // $FlowExpectedError[prop-missing] This property is hidden
            firstNode.comments = [
              node.docblock.comment,
              // $FlowExpectedError[prop-missing]
              ...(firstNode.comments ?? []),
            ];
          }
        }
        return;
      }

      // $FlowExpectedError[prop-missing]
      const comments = node.comments as ?Array<Comment>;
      if (comments != null && comments.length > 0) {
        const leadingComments = [];
        const trailingComments = [];
        for (const comment of comments) {
          // $FlowExpectedError[prop-missing]
          if (comment.trailing === true) {
            trailingComments.push(hermesCommentToBabel(comment));
          } else {
            leadingComments.push(hermesCommentToBabel(comment));
          }
        }

        if (leadingComments.length > 0) {
          // $FlowExpectedError[prop-missing]
          node.leadingComments = leadingComments;
        }
        if (trailingComments.length > 0) {
          // $FlowExpectedError[prop-missing]
          node.trailingComments = trailingComments;
        }
        // $FlowExpectedError[prop-missing]
        delete node.comments;
      }
    },
    leave() {},
  });

  const babelAST = TransformESTreeToBabel.transformProgram(strippedAST, {});

  // Set the filename into each loc for the source map.
  // $FlowExpectedError[incompatible-call]
  SimpleTraverser.traverse(babelAST, {
    // $FlowExpectedError[unclear-type]
    enter(node: any) {
      if (node.loc != null && node.loc.filename == null) {
        node.loc.filename = file;
      }
    },
    leave() {},
    visitorKeys: babelVisitorKeys,
  });

  return babelAST;
}

export async function writeBundle(
  file: string,
  code: string,
  // $FlowExpectedError[unclear-type]
  map: any,
): Promise<void> {
  await writeFile(
    file,
    code + '\n//# sourceMappingURL=' + path.basename(file) + '.map\n',
  );
  await writeFile(file + '.map', JSON.stringify(map));
}
