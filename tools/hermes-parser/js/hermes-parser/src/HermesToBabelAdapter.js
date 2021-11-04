/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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

import HermesASTAdapter from './HermesASTAdapter';

export default class HermesToBabelAdapter extends HermesASTAdapter {
  fixSourceLocation(node: HermesNode): void {
    const loc = node.loc;
    if (loc == null) {
      return;
    }

    node.loc = {
      source: this.sourceFilename ?? null,
      start: loc.start,
      end: loc.end,
    };

    node.start = loc.rangeStart;
    node.end = loc.rangeEnd;
  }

  mapNode(node: HermesNode): HermesNode {
    this.fixSourceLocation(node);
    switch (node.type) {
      case 'Program':
        return this.mapProgram(node);
      case 'BlockStatement':
        return this.mapNodeWithDirectives(node);
      case 'Empty':
        return this.mapEmpty(node);
      case 'Identifier':
        return this.mapIdentifier(node);
      case 'TemplateElement':
        return this.mapTemplateElement(node);
      case 'GenericTypeAnnotation':
        return this.mapGenericTypeAnnotation(node);
      case 'SymbolTypeAnnotation':
        return this.mapSymbolTypeAnnotation(node);
      case 'Property':
        return this.mapProperty(node);
      case 'MethodDefinition':
        return this.mapMethodDefinition(node);
      case 'ImportDeclaration':
        return this.mapImportDeclaration(node);
      case 'ImportSpecifier':
        return this.mapImportSpecifier(node);
      case 'ExportDefaultDeclaration':
        return this.mapExportDefaultDeclaration(node);
      case 'ExportNamedDeclaration':
        return this.mapExportNamedDeclaration(node);
      case 'ExportAllDeclaration':
        return this.mapExportAllDeclaration(node);
      case 'RestElement':
        return this.mapRestElement(node);
      case 'ImportExpression':
        return this.mapImportExpression(node);
      case 'PrivateName':
      case 'ClassPrivateProperty':
        return this.mapPrivateProperty(node);
      case 'FunctionDeclaration':
      case 'FunctionExpression':
        return this.mapFunction(node);
      case 'IndexedAccessType':
      case 'OptionalIndexedAccessType':
        return this.mapUnsupportedTypeAnnotation(node);
      default:
        return this.mapNodeDefault(node);
    }
  }

  mapProgram(node: HermesNode): HermesNode {
    // Visit child nodes and convert to directives
    const {comments, ...program} = this.mapNodeWithDirectives(node);

    program.sourceType = this.getSourceType();

    // Adjust start loc to beginning of file
    program.loc.start = {line: 1, column: 0};
    program.start = 0;

    // Adjust end loc to include last comment if program ends with a comment
    if (comments.length > 0) {
      const lastComment = comments[comments.length - 1];
      if (lastComment.end > program.end) {
        program.loc.end = lastComment.loc.end;
        program.end = lastComment.end;
      }
    }

    // Rename root node to File node and move Program node under program property
    return {
      type: 'File',
      loc: program.loc,
      start: program.start,
      end: program.end,
      program,
      comments,
    };
  }

  mapNodeWithDirectives(node: HermesNode): HermesNode {
    const directives = [];
    for (const child of node.body) {
      if (child.type === 'ExpressionStatement' && child.directive != null) {
        // Visit directive children
        const directiveChild = this.mapNode(child);

        // Modify string literal node to be DirectiveLiteral node
        directiveChild.expression.type = 'DirectiveLiteral';

        // Construct Directive node with DirectiveLiteral value
        directives.push({
          type: 'Directive',
          loc: directiveChild.loc,
          start: directiveChild.start,
          end: directiveChild.end,
          value: directiveChild.expression,
        });
      } else {
        // Once we have found the first non-directive node we know there cannot be any more directives
        break;
      }
    }

    // Move directives from body to new directives array
    node.directives = directives;
    if (directives.length !== 0) {
      node.body = node.body.slice(directives.length);
    }

    // Visit expression statement children
    const body = node.body;
    for (let i = 0; i < body.length; i++) {
      const child = body[i];
      if (child != null) {
        body[i] = this.mapNode(child);
      }
    }

    return node;
  }

  mapIdentifier(node: HermesNode): HermesNode {
    node.loc.identifierName = node.name;
    return this.mapNodeDefault(node);
  }

  mapTemplateElement(node: HermesNode): HermesNode {
    // Adjust start loc to exclude "`" at beginning of template literal if this is the first quasi,
    // otherwise exclude "}" from previous expression.
    const startCharsToExclude = 1;

    // Adjust end loc to exclude "`" at end of template literal if this is the last quasi,
    // otherwise exclude "${" from next expression.
    const endCharsToExclude = node.tail ? 1 : 2;

    return {
      type: 'TemplateElement',
      loc: {
        start: {
          line: node.loc.start.line,
          column: node.loc.start.column + startCharsToExclude,
        },
        end: {
          line: node.loc.end.line,
          column: node.loc.end.column - endCharsToExclude,
        },
      },
      start: node.start + startCharsToExclude,
      end: node.end - endCharsToExclude,
      tail: node.tail,
      value: {
        cooked: node.cooked,
        raw: node.raw,
      },
    };
  }

  mapGenericTypeAnnotation(node: HermesNode): HermesNode {
    // Convert simple `this` generic type to ThisTypeAnnotation
    if (
      node.typeParameters === null &&
      node.id.type === 'Identifier' &&
      node.id.name === 'this'
    ) {
      return {
        type: 'ThisTypeAnnotation',
        loc: node.loc,
        start: node.start,
        end: node.end,
      };
    }

    return this.mapNodeDefault(node);
  }

  mapSymbolTypeAnnotation(node: HermesNode): HermesNode {
    return {
      type: 'GenericTypeAnnotation',
      loc: node.loc,
      start: node.start,
      end: node.end,
      id: {
        type: 'Identifier',
        loc: node.loc,
        start: node.start,
        end: node.end,
        name: 'symbol',
      },
      typeParameters: null,
    };
  }

  mapProperty(node: HermesNode): HermesNode {
    const key = this.mapNode(node.key);
    const value = this.mapNode(node.value);

    // Convert methods, getters, and setters to ObjectMethod nodes
    if (node.method || node.kind !== 'init') {
      // Properties under the FunctionExpression value that should be moved
      // to the ObjectMethod node itself.
      const {
        id,
        params,
        body,
        async,
        generator,
        returnType,
        typeParameters,
        predicate,
      } = value;

      return {
        type: 'ObjectMethod',
        loc: node.loc,
        start: node.start,
        end: node.end,
        // Non getter or setter methods have `kind = method`
        kind: node.kind === 'init' ? 'method' : node.kind,
        computed: node.computed,
        key,
        id,
        params,
        body,
        async,
        generator,
        returnType,
        typeParameters,
        predicate,
      };
    } else {
      // Non-method property nodes should be renamed to ObjectProperty
      node.type = 'ObjectProperty';
      return node;
    }
  }

  mapMethodDefinition(node: HermesNode): HermesNode {
    const key = this.mapNode(node.key);
    const value = this.mapNode(node.value);

    // Properties under the FunctionExpression value that should be moved
    // to the ClassMethod node itself.
    const {
      id,
      params,
      body,
      async,
      generator,
      returnType,
      typeParameters,
      predicate,
    } = value;

    return {
      type: 'ClassMethod',
      loc: node.loc,
      start: node.start,
      end: node.end,
      kind: node.kind,
      computed: node.computed,
      static: node.static,
      key,
      id,
      params,
      body,
      async,
      generator,
      returnType,
      typeParameters,
      predicate,
    };
  }

  mapRestElement(node: HermesNode): HermesNode {
    const restElement = this.mapNodeDefault(node);

    // Hermes puts type annotations on rest elements on the argument node,
    // but Babel expects type annotations on the rest element node itself.
    const annotation = restElement.argument.typeAnnotation;
    if (annotation != null) {
      restElement.typeAnnotation = annotation;
      restElement.argument.typeAnnotation = null;
    }

    return restElement;
  }

  mapImportExpression(node: HermesNode): HermesNode {
    // Babel expects ImportExpression to be structued as a regular
    // CallExpression where the callee is an Import node.
    return {
      type: 'CallExpression',
      loc: node.loc,
      start: node.start,
      end: node.end,
      callee: {
        type: 'Import',
        loc: node.loc,
        start: node.start,
        end: node.end,
      },
      arguments: [this.mapNode(node.source)],
    };
  }

  mapFunction(node: HermesNode): HermesNode {
    // Remove the first parameter if it is a this-type annotation,
    // which is not recognized by Babel.
    if (node.params.length !== 0 && node.params[0].name === 'this') {
      node.params.shift();
    }

    return this.mapNodeDefault(node);
  }

  /**
   * If Babel (the version we target) does not support a type annotation we
   * parse, we need to return some other valid type annotation in its place.
   */
  mapUnsupportedTypeAnnotation(node: HermesNode): HermesNode {
    return {
      type: 'AnyTypeAnnotation',
      loc: node.loc,
      start: node.start,
      end: node.end,
    };
  }
}
