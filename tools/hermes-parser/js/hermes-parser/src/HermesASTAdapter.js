/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

import type {HermesNode} from './HermesAST';
import type {ParserOptions} from './ParserOptions';

import {
  HERMES_AST_VISITOR_KEYS,
  NODE_CHILD,
  NODE_LIST_CHILD,
} from './generated/ParserVisitorKeys';

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

  formatError(node: HermesNode, message: string): string {
    return `${message} (${node.loc.start.line}:${node.loc.start.column})`;
  }

  getBigIntLiteralValue(bigintString: string): {
    bigint: string,
    value: $FlowFixMe /* bigint */,
  } {
    // TODO - once we update flow we can remove this
    declare var BigInt: ?(value: $FlowFixMe) => mixed;

    const bigint = bigintString
      // estree spec is to not have a trailing `n` on this property
      // https://github.com/estree/estree/blob/db962bb417a97effcfe9892f87fbb93c81a68584/es2020.md#bigintliteral
      .replace(/n$/, '')
      // `BigInt` doesn't accept numeric separator and `bigint` property should not include numeric separator
      .replace(/_/, '');
    return {
      bigint,
      // coerce the string to a bigint value if supported by the environment
      value: typeof BigInt === 'function' ? BigInt(bigint) : null,
    };
  }
}
