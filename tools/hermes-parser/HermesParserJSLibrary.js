/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const HermesParserJSLibrary = {
  $JSReferences: {
    // Reference IDs must start at 1 to avoid being confused with the null pointer
    currentID: 1,
    references: {},

    /**
     * Store the given value and return a reference to it.
     */
    store: function(value) {
      const ref = JSReferences.currentID++;
      JSReferences.references[ref] = value;
      return ref;
    },

    /**
     * Lookup and return the stored value for a reference and remove it from the
     * internal references store.
     *
     * If the reference is 0 (corresponding to the null pointer), return null.
     */
    pop: function(ref) {
      if (ref == 0) {
        return null;
      }

      const value = JSReferences.references[ref];
      delete JSReferences.references[ref];

      return value;
    },

    /**
     * Return the string at a given pointer, returning null if the pointer is 0
     * (corresponding to the null pointer).
     */
    getString: function(pointer) {
      if (pointer == 0) {
        return null;
      }

      return UTF8ToString(pointer);
    },
  },

  buildArray: function() {
    return JSReferences.store([]);
  },

  appendToArray: function(arrayRef, elementRef) {
    JSReferences.references[arrayRef].push(JSReferences.pop(elementRef));
  },

  buildSourceLocation: function(
    startLine,
    startCol,
    endLine,
    endCol,
    rangeStart,
    rangeEnd,
  ) {
    return JSReferences.store({
      start: {
        line: startLine,
        col: startCol,
      },
      end: {
        line: endLine,
        col: endCol,
      },
      rangeStart: rangeStart,
      rangeEnd: rangeEnd,
    });
  },

  build_Program: function(loc, body) {
    return JSReferences.store({
      type: 'Program',
      loc: JSReferences.pop(loc),
      body: JSReferences.pop(body),
    });
  },

  build_FunctionExpression: function(
    loc,
    id,
    params,
    body,
    typeParameters,
    returnType,
    generator,
    async,
  ) {
    return JSReferences.store({
      type: 'FunctionExpression',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      params: JSReferences.pop(params),
      body: JSReferences.pop(body),
      typeParameters: JSReferences.pop(typeParameters),
      returnType: JSReferences.pop(returnType),
      generator: Boolean(generator),
      async: Boolean(async),
    });
  },

  build_ArrowFunctionExpression: function(
    loc,
    id,
    params,
    body,
    typeParameters,
    returnType,
    expression,
    async,
  ) {
    return JSReferences.store({
      type: 'ArrowFunctionExpression',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      params: JSReferences.pop(params),
      body: JSReferences.pop(body),
      typeParameters: JSReferences.pop(typeParameters),
      returnType: JSReferences.pop(returnType),
      expression: Boolean(expression),
      async: Boolean(async),
    });
  },

  build_FunctionDeclaration: function(
    loc,
    id,
    params,
    body,
    typeParameters,
    returnType,
    generator,
    async,
  ) {
    return JSReferences.store({
      type: 'FunctionDeclaration',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      params: JSReferences.pop(params),
      body: JSReferences.pop(body),
      typeParameters: JSReferences.pop(typeParameters),
      returnType: JSReferences.pop(returnType),
      generator: Boolean(generator),
      async: Boolean(async),
    });
  },

  build_WhileStatement: function(loc, body, test) {
    return JSReferences.store({
      type: 'WhileStatement',
      loc: JSReferences.pop(loc),
      body: JSReferences.pop(body),
      test: JSReferences.pop(test),
    });
  },

  build_DoWhileStatement: function(loc, body, test) {
    return JSReferences.store({
      type: 'DoWhileStatement',
      loc: JSReferences.pop(loc),
      body: JSReferences.pop(body),
      test: JSReferences.pop(test),
    });
  },

  build_ForInStatement: function(loc, left, right, body) {
    return JSReferences.store({
      type: 'ForInStatement',
      loc: JSReferences.pop(loc),
      left: JSReferences.pop(left),
      right: JSReferences.pop(right),
      body: JSReferences.pop(body),
    });
  },

  build_ForOfStatement: function(loc, left, right, body) {
    return JSReferences.store({
      type: 'ForOfStatement',
      loc: JSReferences.pop(loc),
      left: JSReferences.pop(left),
      right: JSReferences.pop(right),
      body: JSReferences.pop(body),
    });
  },

  build_ForStatement: function(loc, init, test, update, body) {
    return JSReferences.store({
      type: 'ForStatement',
      loc: JSReferences.pop(loc),
      init: JSReferences.pop(init),
      test: JSReferences.pop(test),
      update: JSReferences.pop(update),
      body: JSReferences.pop(body),
    });
  },

  build_DebuggerStatement: function(loc) {
    return JSReferences.store({
      type: 'DebuggerStatement',
      loc: JSReferences.pop(loc),
    });
  },

  build_EmptyStatement: function(loc) {
    return JSReferences.store({
      type: 'EmptyStatement',
      loc: JSReferences.pop(loc),
    });
  },

  build_BlockStatement: function(loc, body) {
    return JSReferences.store({
      type: 'BlockStatement',
      loc: JSReferences.pop(loc),
      body: JSReferences.pop(body),
    });
  },

  build_BreakStatement: function(loc, label) {
    return JSReferences.store({
      type: 'BreakStatement',
      loc: JSReferences.pop(loc),
      label: JSReferences.pop(label),
    });
  },

  build_ContinueStatement: function(loc, label) {
    return JSReferences.store({
      type: 'ContinueStatement',
      loc: JSReferences.pop(loc),
      label: JSReferences.pop(label),
    });
  },

  build_ThrowStatement: function(loc, argument) {
    return JSReferences.store({
      type: 'ThrowStatement',
      loc: JSReferences.pop(loc),
      argument: JSReferences.pop(argument),
    });
  },

  build_ReturnStatement: function(loc, argument) {
    return JSReferences.store({
      type: 'ReturnStatement',
      loc: JSReferences.pop(loc),
      argument: JSReferences.pop(argument),
    });
  },

  build_WithStatement: function(loc, object, body) {
    return JSReferences.store({
      type: 'WithStatement',
      loc: JSReferences.pop(loc),
      object: JSReferences.pop(object),
      body: JSReferences.pop(body),
    });
  },

  build_SwitchStatement: function(loc, discriminant, cases) {
    return JSReferences.store({
      type: 'WithStatement',
      loc: JSReferences.pop(loc),
      discriminant: JSReferences.pop(discriminant),
      cases: JSReferences.pop(cases),
    });
  },

  build_LabeledStatement: function(loc, label, body) {
    return JSReferences.store({
      type: 'LabeledStatement',
      loc: JSReferences.pop(loc),
      label: JSReferences.pop(label),
      body: JSReferences.pop(body),
    });
  },

  build_ExpressionStatement: function(loc, expression, directive) {
    return JSReferences.store({
      type: 'ExpressionStatement',
      loc: JSReferences.pop(loc),
      expression: JSReferences.pop(expression),
      directive: JSReferences.getString(directive),
    });
  },

  build_TryStatement: function(loc, block, handler, finalizer) {
    return JSReferences.store({
      type: 'TryStatement',
      loc: JSReferences.pop(loc),
      block: JSReferences.pop(block),
      handler: JSReferences.pop(handler),
      finalizer: JSReferences.pop(finalizer),
    });
  },

  build_IfStatement: function(loc, test, consequent, alternate) {
    return JSReferences.store({
      type: 'IfStatement',
      loc: JSReferences.pop(loc),
      test: JSReferences.pop(test),
      consequent: JSReferences.pop(consequent),
      alternate: JSReferences.pop(alternate),
    });
  },

  build_NullLiteral: function(loc) {
    return JSReferences.store({
      type: 'Literal',
      loc: JSReferences.pop(loc),
      value: null,
    });
  },

  build_BooleanLiteral: function(loc, value) {
    return JSReferences.store({
      type: 'Literal',
      loc: JSReferences.pop(loc),
      value: Boolean(value),
    });
  },

  build_StringLiteral: function(loc, value) {
    return JSReferences.store({
      type: 'Literal',
      loc: JSReferences.pop(loc),
      value: JSReferences.getString(value),
    });
  },

  build_NumericLiteral: function(loc, value) {
    return JSReferences.store({
      type: 'Literal',
      loc: JSReferences.pop(loc),
      value: value,
    });
  },

  build_RegExpLiteral: function(loc, pattern, flags) {
    return JSReferences.store({
      type: 'Literal',
      loc: JSReferences.pop(loc),
      pattern: JSReferences.getString(pattern),
      flags: JSReferences.getString(flags),
    });
  },

  build_ThisExpression: function(loc) {
    return JSReferences.store({
      type: 'ThisExpression',
      loc: JSReferences.pop(loc),
    });
  },

  build_Super: function(loc) {
    return JSReferences.store({
      type: 'Super',
      loc: JSReferences.pop(loc),
    });
  },

  build_Import: function(loc) {
    return JSReferences.store({
      type: 'Import',
      loc: JSReferences.pop(loc),
    });
  },

  build_SequenceExpression: function(loc, expressions) {
    return JSReferences.store({
      type: 'SequenceExpression',
      loc: JSReferences.pop(loc),
      expressions: JSReferences.pop(expressions),
    });
  },

  build_ObjectExpression: function(loc, properties) {
    return JSReferences.store({
      type: 'ObjectExpression',
      loc: JSReferences.pop(loc),
      properties: JSReferences.pop(properties),
    });
  },

  build_ArrayExpression: function(loc, elements, trailingComma) {
    return JSReferences.store({
      type: 'ArrayExpression',
      loc: JSReferences.pop(loc),
      elements: JSReferences.pop(elements),
    });
  },

  build_SpreadElement: function(loc, argument) {
    return JSReferences.store({
      type: 'SpreadElement',
      loc: JSReferences.pop(loc),
      argument: JSReferences.pop(argument),
    });
  },

  build_NewExpression: function(loc, callee, args) {
    return JSReferences.store({
      type: 'NewExpression',
      loc: JSReferences.pop(loc),
      callee: JSReferences.pop(callee),
      arguments: JSReferences.pop(args),
    });
  },

  build_YieldExpression: function(loc, argument, delegate) {
    return JSReferences.store({
      type: 'YieldExpression',
      loc: JSReferences.pop(loc),
      argument: JSReferences.pop(argument),
      delegate: Boolean(delegate),
    });
  },

  build_AwaitExpression: function(loc, argument) {
    return JSReferences.store({
      type: 'AwaitExpression',
      loc: JSReferences.pop(loc),
      argument: JSReferences.pop(argument),
    });
  },

  build_CallExpression: function(loc, callee, typeArguments, args) {
    return JSReferences.store({
      type: 'CallExpression',
      loc: JSReferences.pop(loc),
      callee: JSReferences.pop(callee),
      typeArguments: JSReferences.pop(typeArguments),
      arguments: JSReferences.pop(args),
    });
  },

  build_OptionalCallExpression: function(
    loc,
    callee,
    typeArguments,
    args,
    optional,
  ) {
    return JSReferences.store({
      type: 'OptionalCallExpression',
      loc: JSReferences.pop(loc),
      callee: JSReferences.pop(callee),
      typeArguments: JSReferences.pop(typeArguments),
      arguments: JSReferences.pop(args),
      optional: Boolean(optional),
    });
  },

  build_AssignmentExpression: function(loc, operator, left, right) {
    return JSReferences.store({
      type: 'AssignmentExpression',
      loc: JSReferences.pop(loc),
      operator: JSReferences.getString(operator),
      left: JSReferences.pop(left),
      right: JSReferences.pop(right),
    });
  },

  build_UnaryExpression: function(loc, operator, argument, prefix) {
    return JSReferences.store({
      type: 'UnaryExpression',
      loc: JSReferences.pop(loc),
      operator: JSReferences.getString(operator),
      argument: JSReferences.pop(argument),
      prefix: Boolean(prefix),
    });
  },

  build_UpdateExpression: function(loc, operator, argument, prefix) {
    return JSReferences.store({
      type: 'UpdateExpression',
      loc: JSReferences.pop(loc),
      operator: JSReferences.getString(operator),
      argument: JSReferences.pop(argument),
      prefix: Boolean(prefix),
    });
  },

  build_MemberExpression: function(loc, object, property, computed) {
    return JSReferences.store({
      type: 'MemberExpression',
      loc: JSReferences.pop(loc),
      object: JSReferences.pop(object),
      property: JSReferences.pop(property),
      computed: Boolean(computed),
    });
  },

  build_OptionalMemberExpression: function(
    loc,
    object,
    property,
    computed,
    optional,
  ) {
    return JSReferences.store({
      type: 'OptionalMemberExpression',
      loc: JSReferences.pop(loc),
      object: JSReferences.pop(object),
      property: JSReferences.pop(property),
      computed: Boolean(computed),
      optional: Boolean(optional),
    });
  },

  build_LogicalExpression: function(loc, left, right, operator) {
    return JSReferences.store({
      type: 'LogicalExpression',
      loc: JSReferences.pop(loc),
      left: JSReferences.pop(left),
      right: JSReferences.pop(right),
      operator: JSReferences.getString(operator),
    });
  },

  build_ConditionalExpression: function(loc, test, alternate, consequent) {
    return JSReferences.store({
      type: 'ConditionalExpression',
      loc: JSReferences.pop(loc),
      test: JSReferences.pop(test),
      alternate: JSReferences.pop(alternate),
      consequent: JSReferences.pop(consequent),
    });
  },

  build_BinaryExpression: function(loc, left, right, operator) {
    return JSReferences.store({
      type: 'BinaryExpression',
      loc: JSReferences.pop(loc),
      left: JSReferences.pop(left),
      right: JSReferences.pop(right),
      operator: JSReferences.getString(operator),
    });
  },

  build_Directive: function(loc, value) {
    return JSReferences.store({
      type: 'Directive',
      loc: JSReferences.pop(loc),
      value: JSReferences.pop(value),
    });
  },

  build_DirectiveLiteral: function(loc, value) {
    return JSReferences.store({
      type: 'DirectiveLiteral',
      loc: JSReferences.pop(loc),
      value: JSReferences.getString(value),
    });
  },

  build_Identifier: function(loc, name, typeAnnotation) {
    return JSReferences.store({
      type: 'Identifier',
      loc: JSReferences.pop(loc),
      name: JSReferences.getString(name),
      typeAnnotation: JSReferences.pop(typeAnnotation),
    });
  },

  build_MetaProperty: function(loc, meta, property) {
    return JSReferences.store({
      type: 'MetaProperty',
      loc: JSReferences.pop(loc),
      meta: JSReferences.pop(meta),
      property: JSReferences.pop(property),
    });
  },

  build_SwitchCase: function(loc, test, consequent) {
    return JSReferences.store({
      type: 'SwitchCase',
      loc: JSReferences.pop(loc),
      test: JSReferences.pop(test),
      consequent: JSReferences.pop(consequent),
    });
  },

  build_CatchClause: function(loc, param, body) {
    return JSReferences.store({
      type: 'SwitchCase',
      loc: JSReferences.pop(loc),
      param: JSReferences.pop(param),
      body: JSReferences.pop(body),
    });
  },

  build_VariableDeclarator: function(loc, init, id) {
    return JSReferences.store({
      type: 'VariableDeclarator',
      loc: JSReferences.pop(loc),
      init: JSReferences.pop(init),
      id: JSReferences.pop(id),
    });
  },

  build_VariableDeclaration: function(loc, kind, declarations) {
    return JSReferences.store({
      type: 'VariableDeclaration',
      loc: JSReferences.pop(loc),
      kind: JSReferences.getString(kind),
      declarations: JSReferences.pop(declarations),
    });
  },

  build_TemplateLiteral: function(loc, quasis, expressions) {
    return JSReferences.store({
      type: 'TemplateLiteral',
      loc: JSReferences.pop(loc),
      quasis: JSReferences.pop(quasis),
      expressions: JSReferences.pop(expressions),
    });
  },

  build_TaggedTemplateExpression: function(loc, tag, quasi) {
    return JSReferences.store({
      type: 'TaggedTemplateExpression',
      loc: JSReferences.pop(loc),
      tag: JSReferences.pop(tag),
      quasi: JSReferences.pop(quasi),
    });
  },

  build_TemplateElement: function(loc, tail, cooked, raw) {
    return JSReferences.store({
      type: 'TemplateElement',
      loc: JSReferences.pop(loc),
      tail: Boolean(tail),
      value: {
        cooked: JSReferences.getString(cooked),
        raw: JSReferences.getString(raw),
      },
    });
  },

  build_Property: function(loc, key, value, kind, computed) {
    return JSReferences.store({
      type: 'Property',
      loc: JSReferences.pop(loc),
      key: JSReferences.pop(key),
      value: JSReferences.pop(value),
      kind: JSReferences.getString(kind),
      computed: Boolean(computed),
    });
  },

  build_ClassDeclaration: function(
    loc,
    id,
    typeParameters,
    superClass,
    superTypeParameters,
    implements,
    decorators,
    body,
  ) {
    return JSReferences.store({
      type: 'ClassDeclaration',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      superClass: JSReferences.pop(superClass),
      superTypeParameters: JSReferences.pop(superTypeParameters),
      implements: JSReferences.pop(implements),
      decorators: JSReferences.pop(decorators),
      body: JSReferences.pop(body),
    });
  },

  build_ClassExpression: function(
    loc,
    id,
    typeParameters,
    superClass,
    superTypeParameters,
    implements,
    decorators,
    body,
  ) {
    return JSReferences.store({
      type: 'ClassExpression',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      superClass: JSReferences.pop(superClass),
      superTypeParameters: JSReferences.pop(superTypeParameters),
      implements: JSReferences.pop(implements),
      decorators: JSReferences.pop(decorators),
      body: JSReferences.pop(body),
    });
  },

  build_ClassBody: function(loc, body) {
    return JSReferences.store({
      type: 'ClassBody',
      loc: JSReferences.pop(loc),
      body: JSReferences.pop(body),
    });
  },

  build_ClassProperty: function(
    loc,
    key,
    value,
    computed,
    static,
    variance,
    typeAnnotation,
  ) {
    return JSReferences.store({
      type: 'ClassProperty',
      loc: JSReferences.pop(loc),
      key: JSReferences.pop(key),
      value: JSReferences.pop(value),
      computed: Boolean(computed),
      static: Boolean(static),
      variance: JSReferences.pop(variance),
      typeAnnotation: JSReferences.pop(typeAnnotation),
    });
  },

  build_MethodDefinition: function(loc, key, value, kind, computed, static) {
    return JSReferences.store({
      type: 'MethodDefinition',
      loc: JSReferences.pop(loc),
      key: JSReferences.pop(key),
      value: JSReferences.pop(value),
      kind: JSReferences.getString(kind),
      computed: Boolean(computed),
      static: Boolean(static),
    });
  },

  build_ImportDeclaration: function(loc, specifiers, source, importKind) {
    return JSReferences.store({
      type: 'ImportDeclaration',
      loc: JSReferences.pop(loc),
      specifiers: JSReferences.pop(specifiers),
      source: JSReferences.pop(source),
      importKind: JSReferences.getString(importKind),
    });
  },

  build_ImportSpecifier: function(loc, imported, local, importKind) {
    return JSReferences.store({
      type: 'ImportSpecifier',
      loc: JSReferences.pop(loc),
      imported: JSReferences.pop(imported),
      local: JSReferences.pop(local),
      importKind: JSReferences.getString(importKind),
    });
  },

  build_ImportDefaultSpecifier: function(loc, local) {
    return JSReferences.store({
      type: 'ImportDefaultSpecifier',
      loc: JSReferences.pop(loc),
      local: JSReferences.pop(local),
    });
  },

  build_ImportNamespaceSpecifier: function(loc, local) {
    return JSReferences.store({
      type: 'ImportNamespaceSpecifier',
      loc: JSReferences.pop(loc),
      local: JSReferences.pop(local),
    });
  },

  build_ExportNamedDeclaration: function(
    loc,
    declaration,
    specifiers,
    source,
    exportKind,
  ) {
    return JSReferences.store({
      type: 'ExportNamedDeclaration',
      loc: JSReferences.pop(loc),
      declaration: JSReferences.pop(declaration),
      specifiers: JSReferences.pop(specifiers),
      source: JSReferences.pop(source),
      exportKind: JSReferences.getString(exportKind),
    });
  },

  build_ExportSpecifier: function(loc, exported, local) {
    return JSReferences.store({
      type: 'ExportSpecifier',
      loc: JSReferences.pop(loc),
      exported: JSReferences.pop(exported),
      local: JSReferences.pop(local),
    });
  },

  build_ExportDefaultDeclaration: function(loc, declaration) {
    return JSReferences.store({
      type: 'ExportDefaultDeclaration',
      loc: JSReferences.pop(loc),
      declaration: JSReferences.pop(declaration),
    });
  },

  build_ExportAllDeclaration: function(loc, source, exportKind) {
    return JSReferences.store({
      type: 'ExportAllDeclaration',
      loc: JSReferences.pop(loc),
      source: JSReferences.pop(source),
      exportKind: JSReferences.getString(exportKind),
    });
  },

  build_ObjectPattern: function(loc, properties, typeAnnotation) {
    return JSReferences.store({
      type: 'ObjectPattern',
      loc: JSReferences.pop(loc),
      properties: JSReferences.pop(properties),
      typeAnnotation: JSReferences.pop(typeAnnotation),
    });
  },

  build_ArrayPattern: function(loc, elements, typeAnnotation) {
    return JSReferences.store({
      type: 'ArrayPattern',
      loc: JSReferences.pop(loc),
      elements: JSReferences.pop(elements),
      typeAnnotation: JSReferences.pop(typeAnnotation),
    });
  },

  build_RestElement: function(loc, argument) {
    return JSReferences.store({
      type: 'RestElement',
      loc: JSReferences.pop(loc),
      argument: JSReferences.pop(argument),
    });
  },

  build_AssignmentPattern: function(loc, left, right) {
    return JSReferences.store({
      type: 'AssignmentPattern',
      loc: JSReferences.pop(loc),
      left: JSReferences.pop(left),
      right: JSReferences.pop(right),
    });
  },

  build_JSXIdentifier: function(loc, name) {
    return JSReferences.store({
      type: 'JSXIdentifier',
      loc: JSReferences.pop(loc),
      name: JSReferences.getString(name),
    });
  },

  build_JSXMemberExpression: function(loc, object, property) {
    return JSReferences.store({
      type: 'JSXMemberExpression',
      loc: JSReferences.pop(loc),
      object: JSReferences.pop(object),
      property: JSReferences.pop(property),
    });
  },

  build_JSXNamespacedName: function(loc, namespace, name) {
    return JSReferences.store({
      type: 'JSXNamespacedName',
      loc: JSReferences.pop(loc),
      namespace: JSReferences.pop(namespace),
      name: JSReferences.pop(name),
    });
  },

  build_JSXEmptyExpression: function(loc) {
    return JSReferences.store({
      type: 'JSXEmptyExpression',
      loc: JSReferences.pop(loc),
    });
  },

  build_JSXExpressionContainer: function(loc, expression) {
    return JSReferences.store({
      type: 'JSXExpressionContainer',
      loc: JSReferences.pop(loc),
      expression: JSReferences.pop(expression),
    });
  },

  build_JSXSpreadChild: function(loc, expression) {
    return JSReferences.store({
      type: 'JSXSpreadChild',
      loc: JSReferences.pop(loc),
      expression: JSReferences.pop(expression),
    });
  },

  build_JSXOpeningElement: function(loc, name, attributes, selfClosing) {
    return JSReferences.store({
      type: 'JSXOpeningElement',
      loc: JSReferences.pop(loc),
      name: JSReferences.pop(name),
      attributes: JSReferences.pop(attributes),
      selfClosing: Boolean(selfClosing),
    });
  },

  build_JSXClosingElement: function(loc, name) {
    return JSReferences.store({
      type: 'JSXClosingElement',
      loc: JSReferences.pop(loc),
      name: JSReferences.pop(name),
    });
  },

  build_JSXAttribute: function(loc, name, value) {
    return JSReferences.store({
      type: 'JSXAttribute',
      loc: JSReferences.pop(loc),
      name: JSReferences.pop(name),
      value: JSReferences.pop(value),
    });
  },

  build_JSXSpreadAttribute: function(loc, argument) {
    return JSReferences.store({
      loc: JSReferences.pop(loc),
      type: 'JSXSpreadAttribute',
      argument: JSReferences.pop(argument),
    });
  },

  build_JSXText: function(loc, value, raw) {
    return JSReferences.store({
      loc: JSReferences.pop(loc),
      type: 'JSXText',
      value: JSReferences.getString(value),
      raw: JSReferences.getString(raw),
    });
  },

  build_JSXElement: function(loc, openingElement, children, closingElement) {
    return JSReferences.store({
      type: 'JSXElement',
      loc: JSReferences.pop(loc),
      openingElement: JSReferences.pop(openingElement),
      children: JSReferences.pop(children),
      closingElement: JSReferences.pop(closingElement),
    });
  },

  build_JSXFragment: function(loc, openingFragment, children, closingFragment) {
    return JSReferences.store({
      type: 'JSXFragment',
      loc: JSReferences.pop(loc),
      openingFragment: JSReferences.pop(openingFragment),
      children: JSReferences.pop(children),
      closingFragment: JSReferences.pop(closingFragment),
    });
  },

  build_JSXOpeningFragment: function(loc) {
    return JSReferences.store({
      type: 'JSXOpeningFragment',
      loc: JSReferences.pop(loc),
    });
  },

  build_JSXClosingFragment: function(loc) {
    return JSReferences.store({
      type: 'JSXClosingFragment',
      loc: JSReferences.pop(loc),
    });
  },

  build_ExistsTypeAnnotation: function(loc) {
    return JSReferences.store({
      type: 'ExistsTypeAnnotation',
      loc: JSReferences.pop(loc),
    });
  },

  build_EmptyTypeAnnotation: function(loc) {
    return JSReferences.store({
      type: 'EmptyTypeAnnotation',
      loc: JSReferences.pop(loc),
    });
  },

  build_StringTypeAnnotation: function(loc) {
    return JSReferences.store({
      type: 'StringTypeAnnotation',
      loc: JSReferences.pop(loc),
    });
  },

  build_NumberTypeAnnotation: function(loc) {
    return JSReferences.store({
      type: 'NumberTypeAnnotation',
      loc: JSReferences.pop(loc),
    });
  },

  build_NumberLiteralTypeAnnotation: function(loc, value, raw) {
    return JSReferences.store({
      type: 'NumberLiteralTypeAnnotation',
      loc: JSReferences.pop(loc),
      value: value,
      raw: JSReferences.getString(raw),
    });
  },

  build_BooleanTypeAnnotation: function(loc) {
    return JSReferences.store({
      type: 'BooleanTypeAnnotation',
      loc: JSReferences.pop(loc),
    });
  },

  build_BooleanLiteralTypeAnnotation: function(loc, value, raw) {
    return JSReferences.store({
      type: 'BooleanLiteralTypeAnnotation',
      loc: JSReferences.pop(loc),
      value: Boolean(value),
      raw: JSReferences.getString(raw),
    });
  },

  build_NullLiteralTypeAnnotation: function(loc) {
    return JSReferences.store({
      type: 'NullLiteralTypeAnnotation',
      loc: JSReferences.pop(loc),
    });
  },

  build_AnyTypeAnnotation: function(loc) {
    return JSReferences.store({
      type: 'AnyTypeAnnotation',
      loc: JSReferences.pop(loc),
    });
  },

  build_VoidTypeAnnotation: function(loc) {
    return JSReferences.store({
      type: 'VoidTypeAnnotation',
      loc: JSReferences.pop(loc),
    });
  },

  build_FunctionTypeAnnotation: function(
    loc,
    params,
    returnType,
    rest,
    typeParameters,
  ) {
    return JSReferences.store({
      type: 'FunctionTypeAnnotation',
      loc: JSReferences.pop(loc),
      params: JSReferences.pop(params),
      returnType: JSReferences.pop(returnType),
      rest: JSReferences.pop(rest),
      typeParameters: JSReferences.pop(typeParameters),
    });
  },

  build_FunctionTypeParam: function(loc, name, typeAnnotation, optional) {
    return JSReferences.store({
      type: 'FunctionTypeParam',
      loc: JSReferences.pop(loc),
      name: JSReferences.pop(name),
      typeAnnotation: JSReferences.pop(typeAnnotation),
      optional: Boolean(optional),
    });
  },

  build_NullableTypeAnnotation: function(loc, typeAnnotation) {
    return JSReferences.store({
      type: 'NullableTypeAnnotation',
      loc: JSReferences.pop(loc),
      typeAnnotation: JSReferences.pop(typeAnnotation),
    });
  },

  build_QualifiedTypeIdentifier: function(loc, qualification, id) {
    return JSReferences.store({
      type: 'QualifiedTypeIdentifier',
      loc: JSReferences.pop(loc),
      qualification: JSReferences.pop(qualification),
      id: JSReferences.pop(id),
    });
  },

  build_TypeofTypeAnnotation: function(loc, argument) {
    return JSReferences.store({
      type: 'TypeofTypeAnnotation',
      loc: JSReferences.pop(loc),
      argument: JSReferences.pop(argument),
    });
  },

  build_TupleTypeAnnotation: function(loc, types) {
    return JSReferences.store({
      type: 'TupleTypeAnnotation',
      loc: JSReferences.pop(loc),
      types: JSReferences.pop(types),
    });
  },

  build_ArrayTypeAnnotation: function(loc, elementType) {
    return JSReferences.store({
      type: 'ArrayTypeAnnotation',
      loc: JSReferences.pop(loc),
      elementType: JSReferences.pop(elementType),
    });
  },

  build_UnionTypeAnnotation: function(loc, types) {
    return JSReferences.store({
      type: 'UnionTypeAnnotation',
      loc: JSReferences.pop(loc),
      types: JSReferences.pop(types),
    });
  },

  build_IntersectionTypeAnnotation: function(loc, types) {
    return JSReferences.store({
      type: 'IntersectionTypeAnnotation',
      loc: JSReferences.pop(loc),
      types: JSReferences.pop(types),
    });
  },

  build_GenericTypeAnnotation: function(loc, id, typeParameters) {
    return JSReferences.store({
      type: 'GenericTypeAnnotation',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
    });
  },

  build_InterfaceTypeAnnotation: function(loc, extends_, body) {
    return JSReferences.store({
      type: 'InterfaceTypeAnnotation',
      loc: JSReferences.pop(loc),
      extends: JSReferences.pop(extends_),
      body: JSReferences.pop(body),
    });
  },

  build_TypeAlias: function(loc, id, typeParameters, right) {
    return JSReferences.store({
      type: 'TypeAlias',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      right: JSReferences.pop(right),
    });
  },

  build_OpaqueType: function(loc, id, typeParameters, impltype, supertype) {
    return JSReferences.store({
      type: 'OpaqueType',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      impltype: JSReferences.pop(impltype),
      supertype: JSReferences.pop(supertype),
    });
  },

  build_InterfaceDeclaration: function(
    loc,
    id,
    typeParameters,
    extends_,
    body,
  ) {
    return JSReferences.store({
      type: 'InterfaceDeclaration',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      extends: JSReferences.pop(extends_),
      body: JSReferences.pop(body),
    });
  },

  build_DeclareTypeAlias: function(loc, id, typeParameters, right) {
    return JSReferences.store({
      type: 'DeclareTypeAlias',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      right: JSReferences.pop(right),
    });
  },

  build_DeclareOpaqueType: function(
    loc,
    id,
    typeParameters,
    impltype,
    supertype,
  ) {
    return JSReferences.store({
      type: 'DeclareOpaqueType',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      impltype: JSReferences.pop(impltype),
      supertype: JSReferences.pop(supertype),
    });
  },

  build_DeclareInterface: function(loc, id, typeParameters, extends_, body) {
    return JSReferences.store({
      type: 'DeclareInterface',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      extends: JSReferences.pop(extends_),
      body: JSReferences.pop(body),
    });
  },

  build_DeclareClass: function(
    loc,
    id,
    typeParameters,
    extends_,
    implements,
    mixins,
    body,
  ) {
    return JSReferences.store({
      type: 'DeclareClass',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
      extends: JSReferences.pop(extends_),
      implements: JSReferences.pop(implements),
      mixins: JSReferences.pop(mixins),
      body: JSReferences.pop(body),
    });
  },

  build_DeclareFunction: function(loc, id, predicate) {
    return JSReferences.store({
      type: 'DeclareFunction',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      predicate: JSReferences.pop(predicate),
    });
  },

  build_DeclareVariable: function(loc, id) {
    return JSReferences.store({
      type: 'DeclareVariable',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
    });
  },

  build_DeclareExportDeclaration: function(
    loc,
    declaration,
    specifiers,
    source,
    default_,
  ) {
    return JSReferences.store({
      type: 'DeclareExportDeclaration',
      loc: JSReferences.pop(loc),
      declaration: JSReferences.pop(declaration),
      specifiers: JSReferences.pop(specifiers),
      source: JSReferences.pop(source),
      default: Boolean(default_),
    });
  },

  build_DeclareExportAllDeclaration: function(loc, source) {
    return JSReferences.store({
      type: 'DeclareExportAllDeclaration',
      loc: JSReferences.pop(loc),
      source: JSReferences.pop(source),
    });
  },

  build_DeclareModule: function(loc, id, body, kind) {
    return JSReferences.store({
      type: 'DeclareModule',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      body: JSReferences.pop(body),
      kind: JSReferences.getString(kind),
    });
  },

  build_DeclareModuleExports: function(loc, typeAnnotation) {
    return JSReferences.store({
      type: 'DeclareModuleExports',
      loc: JSReferences.pop(loc),
      typeAnnotation: JSReferences.pop(typeAnnotation),
    });
  },

  build_InterfaceExtends: function(loc, id, typeParameters) {
    return JSReferences.store({
      type: 'InterfaceExtends',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
    });
  },

  build_ClassImplements: function(loc, id, typeParameters) {
    return JSReferences.store({
      type: 'ClassImplements',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
    });
  },

  build_ClassImplements: function(loc, id, typeParameters) {
    return JSReferences.store({
      type: 'ClassImplements',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      typeParameters: JSReferences.pop(typeParameters),
    });
  },

  build_TypeAnnotation: function(loc, typeAnnotation) {
    return JSReferences.store({
      type: 'TypeAnnotation',
      loc: JSReferences.pop(loc),
      typeAnnotation: JSReferences.pop(typeAnnotation),
    });
  },

  build_ObjectTypeAnnotation: function(
    loc,
    properties,
    indexers,
    callProperties,
    internalSlots,
    inexact,
    exact,
  ) {
    return JSReferences.store({
      type: 'ObjectTypeAnnotation',
      loc: JSReferences.pop(loc),
      properties: JSReferences.pop(properties),
      indexers: JSReferences.pop(indexers),
      callProperties: JSReferences.pop(callProperties),
      internalSlots: JSReferences.pop(internalSlots),
      inexact: Boolean(inexact),
      exact: Boolean(exact),
    });
  },

  build_ObjectTypeProperty: function(
    loc,
    key,
    value,
    method,
    optional,
    static,
    proto,
    variance,
    kind,
  ) {
    return JSReferences.store({
      type: 'ObjectTypeProperty',
      loc: JSReferences.pop(loc),
      key: JSReferences.pop(key),
      value: JSReferences.pop(value),
      method: Boolean(method),
      optional: Boolean(optional),
      static: Boolean(static),
      proto: Boolean(proto),
      variance: JSReferences.pop(variance),
      kind: JSReferences.getString(kind),
    });
  },

  build_ObjectTypeSpreadProperty: function(loc, argument) {
    return JSReferences.store({
      type: 'ObjectTypeSpreadProperty',
      loc: JSReferences.pop(loc),
      argument: JSReferences.pop(argument),
    });
  },

  build_ObjectTypeInternalSlot: function(
    loc,
    id,
    value,
    optional,
    static,
    method,
  ) {
    return JSReferences.store({
      type: 'ObjectTypeInternalSlot',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      value: JSReferences.pop(value),
      optional: Boolean(optional),
      static: Boolean(static),
      method: Boolean(method),
    });
  },

  build_ObjectTypeCallProperty: function(loc, value, static) {
    return JSReferences.store({
      type: 'ObjectTypeCallProperty',
      loc: JSReferences.pop(loc),
      value: JSReferences.pop(value),
      static: Boolean(static),
    });
  },

  build_ObjectTypeIndexer: function(loc, id, key, value, static, variance) {
    return JSReferences.store({
      type: 'ObjectTypeIndexer',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      key: JSReferences.pop(key),
      value: JSReferences.pop(value),
      static: Boolean(static),
      variance: JSReferences.pop(variance),
    });
  },

  build_Variance: function(loc, kind) {
    return JSReferences.store({
      type: 'Variance',
      loc: JSReferences.pop(loc),
      kind: JSReferences.getString(kind),
    });
  },

  build_TypeParameterDeclaration: function(loc, params) {
    return JSReferences.store({
      type: 'TypeParameterDeclaration',
      loc: JSReferences.pop(loc),
      params: JSReferences.pop(params),
    });
  },

  build_TypeParameter: function(loc, name, bound, variance, default_) {
    return JSReferences.store({
      type: 'TypeParameter',
      loc: JSReferences.pop(loc),
      name: JSReferences.getString(name),
      bound: JSReferences.pop(bound),
      variance: JSReferences.pop(variance),
      default: JSReferences.pop(default_),
    });
  },

  build_TypeParameterInstantiation: function(loc, params) {
    return JSReferences.store({
      type: 'TypeParameterInstantiation',
      loc: JSReferences.pop(loc),
      params: JSReferences.pop(params),
    });
  },

  build_TypeCastExpression: function(loc, expression, typeAnnotation) {
    return JSReferences.store({
      type: 'TypeCastExpression',
      loc: JSReferences.pop(loc),
      expression: JSReferences.pop(expression),
      typeAnnotation: JSReferences.pop(typeAnnotation),
    });
  },

  build_InferredPredicate: function(loc) {
    return JSReferences.store({
      type: 'InferredPredicate',
      loc: JSReferences.pop(loc),
    });
  },

  build_DeclaredPredicate: function(loc, value) {
    return JSReferences.store({
      type: 'DeclaredPredicate',
      loc: JSReferences.pop(loc),
      value: JSReferences.pop(value),
    });
  },

  build_EnumDeclaration: function(loc, id, body) {
    return JSReferences.store({
      type: 'EnumDeclaration',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      body: JSReferences.pop(body),
    });
  },

  build_EnumStringBody: function(loc, members, explicitType) {
    return JSReferences.store({
      type: 'EnumStringBody',
      loc: JSReferences.pop(loc),
      members: JSReferences.pop(members),
      explicitType: Boolean(explicitType),
    });
  },

  build_EnumNumberBody: function(loc, members, explicitType) {
    return JSReferences.store({
      type: 'EnumNumberBody',
      loc: JSReferences.pop(loc),
      members: JSReferences.pop(members),
      explicitType: Boolean(explicitType),
    });
  },

  build_EnumBooleanBody: function(loc, members, explicitType) {
    return JSReferences.store({
      type: 'EnumBooleanBody',
      loc: JSReferences.pop(loc),
      members: JSReferences.pop(members),
      explicitType: Boolean(explicitType),
    });
  },

  build_EnumSymbolBody: function(loc, members, explicitType) {
    return JSReferences.store({
      type: 'EnumSymbolBody',
      loc: JSReferences.pop(loc),
      members: JSReferences.pop(members),
    });
  },

  build_EnumDefaultedMember: function(loc, id) {
    return JSReferences.store({
      type: 'EnumDefaultedMember',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
    });
  },

  build_EnumStringMember: function(loc, id, init) {
    return JSReferences.store({
      type: 'EnumStringMember',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      init: JSReferences.pop(init),
    });
  },

  build_EnumNumberMember: function(loc, id, init) {
    return JSReferences.store({
      type: 'EnumNumberMember',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      init: JSReferences.pop(init),
    });
  },

  build_EnumBooleanMember: function(loc, id, init) {
    return JSReferences.store({
      type: 'EnumBooleanMember',
      loc: JSReferences.pop(loc),
      id: JSReferences.pop(id),
      init: Boolean(init),
    });
  },

  // Empty node used for holes in arrays, return reference that resolves to null.
  build_Empty: function(loc) {
    JSReferences.pop(loc);
    return 0;
  },

  // Included in Hermes' AST definition but should not be a part of final AST
  build_Metadata: function() {},
  build_CoverEmptyArgs: function() {},
  build_CoverInitializer: function() {},
  build_CoverRestElement: function() {},
  build_CoverTrailingComma: function() {},
  build_CoverTypedIdentifier: function() {},
};

autoAddDeps(HermesParserJSLibrary, '$JSReferences');

mergeInto(LibraryManager.library, HermesParserJSLibrary);
