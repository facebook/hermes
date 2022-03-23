/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

/*
This class does some very "javascripty" things in the name of
performance which are ultimately impossible to soundly type.

So instead of adding strict types and a large number of suppression
comments, instead it is left untyped and subclasses are strictly
typed via a separate flow declaration file.
*/

import type {HermesNode} from './HermesAST';
import type {ParserOptions} from './ParserOptions';

import {
  HERMES_AST_VISITOR_KEYS,
  NODE_CHILD,
  NODE_LIST_CHILD,
} from './generated/visitor-keys';

/**
 * The base class for transforming the Hermes AST to the desired output format.
 * Extended by concrete adapters which output an ESTree or Babel AST.
 */
export default class HermesASTAdapter {
  sourceFilename: ParserOptions['sourceFilename'];
  sourceType: ParserOptions['sourceType'];

  constructor(options: ParserOptions) {
    this.sourceFilename = options.sourceFilename;
    this.sourceType = options.sourceType;
  }

  /**
   * Transform the input Hermes AST to the desired output format.
   * This modifies the input AST in place instead of constructing a new AST.
   */
  transform(program: HermesNode): ?HermesNode {
    // Comments are not traversed via visitor keys
    const comments = program.comments;
    for (let i = 0; i < comments.length; i++) {
      const comment = comments[i];
      this.fixSourceLocation(comment);
      comments[i] = this.mapComment(comment);
    }

    // The first comment may be an interpreter directive and is stored directly on the program node
    program.interpreter =
      comments.length > 0 && comments[0].type === 'InterpreterDirective'
        ? comments.shift()
        : null;

    // Tokens are not traversed via visitor keys
    const tokens = program.tokens;
    if (tokens) {
      for (let i = 0; i < tokens.length; i++) {
        this.fixSourceLocation(tokens[i]);
      }
    }

    return this.mapNode(program);
  }

  /**
   * Transform a Hermes AST node to the output AST format.
   *
   * This may modify the input node in-place and return that same node, or a completely
   * new node may be constructed and returned. Overriden in child classes.
   */
  mapNode(_node: HermesNode): HermesNode {
    throw new Error('Implemented in subclasses');
  }

  mapNodeDefault(node: HermesNode): HermesNode {
    const visitorKeys = HERMES_AST_VISITOR_KEYS[node.type];
    for (const key in visitorKeys) {
      const childType = visitorKeys[key];
      if (childType === NODE_CHILD) {
        const child = node[key];
        if (child != null) {
          node[key] = this.mapNode(child);
        }
      } else if (childType === NODE_LIST_CHILD) {
        const children = node[key];
        for (let i = 0; i < children.length; i++) {
          const child = children[i];
          if (child != null) {
            children[i] = this.mapNode(child);
          }
        }
      }
    }

    return node;
  }

  /**
   * Update the source location for this node depending on the output AST format.
   * This can modify the input node in-place. Overriden in child classes.
   */
  fixSourceLocation(_node: HermesNode): void {
    throw new Error('Implemented in subclasses');
  }

  getSourceType(): ParserOptions['sourceType'] {
    return this.sourceType ?? 'script';
  }

  setModuleSourceType(): void {
    if (this.sourceType == null) {
      this.sourceType = 'module';
    }
  }

  mapComment(node: HermesNode): HermesNode {
    return node;
  }

  mapEmpty(_node: HermesNode): HermesNode {
    // $FlowExpectedError
    return null;
  }

  mapImportDeclaration(node: HermesNode): HermesNode {
    if (node.importKind === 'value') {
      this.setModuleSourceType();
    }

    return this.mapNodeDefault(node);
  }

  mapImportSpecifier(node: HermesNode): HermesNode {
    if (node.importKind === 'value') {
      node.importKind = null;
    }

    return this.mapNodeDefault(node);
  }

  mapExportDefaultDeclaration(node: HermesNode): HermesNode {
    this.setModuleSourceType();
    return this.mapNodeDefault(node);
  }

  mapExportNamedDeclaration(node: HermesNode): HermesNode {
    if (node.exportKind === 'value') {
      this.setModuleSourceType();
    }

    return this.mapNodeDefault(node);
  }

  mapExportAllDeclaration(node: HermesNode): HermesNode {
    if (node.exportKind === 'value') {
      this.setModuleSourceType();
    }

    return this.mapNodeDefault(node);
  }

  mapPrivateProperty(node: HermesNode): HermesNode {
    throw new SyntaxError(
      this.formatError(node, 'Private properties are not supported'),
    );
  }

  formatError(node: HermesNode, message: string): string {
    return `${message} (${node.loc.start.line}:${node.loc.start.column})`;
  }
}
