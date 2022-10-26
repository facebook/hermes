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

import type {ESNode, Program} from 'hermes-estree';
import type {TransformVisitor} from './transform';
import type {RemoveCommentMutation} from './mutations/RemoveComment';

import {parseForESLint} from 'hermes-eslint';
import {updateAllParentPointers} from '../detachedNode';
import {traverseWithContext} from '../traverse/traverse';
import {MutationContext} from './MutationContext';
import {getTransformContext} from './TransformContext';
import {
  addCommentsToNode,
  attachComments,
  getLeadingCommentsForNode,
} from './comments/comments';
import {performAddCommentsMutation} from './mutations/AddComments';
import {performCloneCommentsToMutation} from './mutations/CloneCommentsTo';
import {performInsertStatementMutation} from './mutations/InsertStatement';
import {performRemoveCommentMutations} from './mutations/RemoveComment';
import {performRemoveNodeMutation} from './mutations/RemoveNode';
import {performRemoveStatementMutation} from './mutations/RemoveStatement';
import {performReplaceNodeMutation} from './mutations/ReplaceNode';
import {performReplaceStatementWithManyMutation} from './mutations/ReplaceStatementWithMany';

export function getTransformedAST(
  code: string,
  visitors: TransformVisitor,
): {
  ast: Program,
  astWasMutated: boolean,
  mutatedCode: string,
} {
  const {ast, scopeManager} = parseForESLint(code, {
    sourceType: 'module',
  });

  // Don't include the docblock comment in the comment list as we don't want to attach it
  // as it should be maintained at the top of the file as nodes are moved around.
  let comments = ast.comments;
  if (ast.docblock != null && ast.docblock.comment === comments[0]) {
    const [first, ...nonDocblockComments] = comments;

    // Since we will not be attaching this comment automatically we need to add the
    // properties prettier expects for printing.
    // $FlowExpectedError[prop-missing]
    first.placement = 'endOfLine';
    // $FlowExpectedError[prop-missing]
    first.leading = true;
    // $FlowExpectedError[prop-missing]
    first.trailing = false;
    // $FlowExpectedError[prop-missing]
    first.printed = false;

    comments = nonDocblockComments;
  }

  // attach comments before mutation. this will ensure that as nodes are
  // cloned / moved around - comments remain in the correct place with respect to the node
  attachComments(comments, ast, code);

  // traverse the AST and colllect the mutations
  const transformContext = getTransformContext();
  traverseWithContext(
    code,
    ast,
    scopeManager,
    () => transformContext,
    visitors,
  );

  // apply the mutations to the AST
  const mutationContext = new MutationContext(code);

  const removeCommentMutations: Array<RemoveCommentMutation> = [];

  for (const mutation of transformContext.mutations) {
    const mutationRoot = ((): ESNode | null => {
      switch (mutation.type) {
        case 'insertStatement': {
          return performInsertStatementMutation(mutationContext, mutation);
        }

        case 'replaceNode': {
          return performReplaceNodeMutation(mutationContext, mutation);
        }

        case 'replaceStatementWithMany': {
          return performReplaceStatementWithManyMutation(
            mutationContext,
            mutation,
          );
        }

        case 'removeNode': {
          return performRemoveNodeMutation(mutationContext, mutation);
        }

        case 'removeStatement': {
          return performRemoveStatementMutation(mutationContext, mutation);
        }

        case 'removeComment': {
          // these are handled later
          removeCommentMutations.push(mutation);
          return null;
        }

        case 'addComments': {
          return performAddCommentsMutation(mutationContext, mutation);
        }

        case 'cloneCommentsTo': {
          return performCloneCommentsToMutation(mutationContext, mutation);
        }
      }
    })();

    // ensure the subtree's parent pointers are correct
    // this is required for two reasons:
    // 1) The userland transform is just JS - so there's nothing stopping them
    //    from doing anything dodgy. The flow types have some enforcement, but
    //    ofc that can just be ignored with a suppression.
    // 2) Shallow clones are a necessary evil in the transform because they
    //    allow codemods to do simple changes to just one node without the
    //    weight that comes with deeply cloning the entire AST.
    //    However we can't update the parent pointers of the cloned node's
    //    children until the mutation step or else we would be mutating
    //    real AST nodes and potentially break the traverse step.
    //
    // Being strict here just helps us ensure we keep everything in sync
    if (mutationRoot) {
      updateAllParentPointers(mutationRoot);
    }
  }

  // remove the comments
  // this is done at the end because it requires a complete traversal of the AST
  // so that we can find relevant node's attachment array
  performRemoveCommentMutations(ast, removeCommentMutations);

  // The docblock comment is never attached to any AST nodes, since its technically
  // attached to the program. However this is specific to our AST and in order for
  // prettier to correctly print it we need to attach it to the first node in the
  // program body.
  if (ast.docblock != null && ast.body.length > 0) {
    const firstNode = ast.body[0];
    const docblockComment = ast.docblock.comment;
    const leadingComments = getLeadingCommentsForNode(firstNode);
    if (!leadingComments.includes(docblockComment)) {
      addCommentsToNode(firstNode, [docblockComment], 'leading');
    }
  }

  return {
    ast,
    astWasMutated: transformContext.astWasMutated,
    mutatedCode: mutationContext.code,
  };
}
