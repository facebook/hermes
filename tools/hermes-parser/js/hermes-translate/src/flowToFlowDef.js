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

import type {
  AFunction,
  BindingName,
  ClassBody,
  ClassDeclaration,
  ClassMember,
  DeclareClass,
  DeclareFunction,
  DeclareOpaqueType,
  DeclareVariable,
  ESNode,
  ExportDefaultDeclaration,
  ExportNamedDeclaration,
  Expression,
  FunctionDeclaration,
  FunctionParameter,
  FunctionTypeAnnotation,
  FunctionTypeParam,
  ImportDeclaration,
  InterfaceDeclaration,
  InterfaceExtends,
  Literal,
  ModuleDeclaration,
  ObjectExpression,
  ObjectTypeAnnotation,
  ObjectTypeProperty,
  OpaqueType,
  Program,
  Statement,
  TypeAlias,
  TypeAnnotation,
  TypeAnnotationType,
  TypeCastExpression,
  TypeParameterDeclaration,
  TypeParameterInstantiation,
  VariableDeclaration,
} from 'hermes-estree';
import type {ScopeManager} from 'hermes-eslint';
import type {DetachedNode} from 'hermes-transform';
import type {
  Dep,
  TranslationContext,
  TranslationOptions,
} from './utils/TranslationUtils';

import {t} from 'hermes-transform';
import {
  analyzeFunctionReturn,
  analyzeTypeDependencies,
} from './utils/FlowAnalyze';
import {createTranslationContext} from './utils/TranslationUtils';
import {asDetachedNode} from 'hermes-transform';
import {translationError, flowFixMeOrError} from './utils/ErrorUtils';
import {isExpression} from 'hermes-estree';

const EMPTY_TRANSLATION_RESULT = [null, []];

type TranslatedDeps = $ReadOnlyArray<Dep>;
type TranslatedResultOrNull<T> = [DetachedNode<T> | null, TranslatedDeps];
type TranslatedResultArray<T> = [
  $ReadOnlyArray<DetachedNode<T>>,
  TranslatedDeps,
];
type TranslatedResult<T> = [DetachedNode<T>, TranslatedDeps];

type ProgramStatement = Statement | ModuleDeclaration;

function convertArray<TIn, TOut>(
  items: $ReadOnlyArray<TIn>,
  convert: TIn => TranslatedResultOrNull<TOut>,
): TranslatedResultArray<TOut> {
  const resultItems: Array<DetachedNode<TOut>> = [];
  const deps: Array<Dep> = [];
  for (const item of items) {
    const [resultItem, itemDeps] = convert(item);
    if (resultItem != null) {
      resultItems.push(resultItem);

      deps.push(...itemDeps);
    }
  }
  return [resultItems, deps];
}

function getTopLevelStatement(
  node: ESNode,
  context: TranslationContext,
): ProgramStatement {
  let currentNode: ?ESNode = node;
  while (currentNode != null) {
    if (currentNode.parent?.type === 'Program') {
      // $FlowFixMe[incompatible-return]
      return currentNode;
    }
    currentNode = currentNode.parent;
  }

  throw translationError(
    node,
    `getTopLevelStatement: Detached node of type "${node.type}" passed`,
    context,
  );
}

function transferProgramStatementProperties(
  stmt: DetachedNode<ProgramStatement>,
  orgStmt: ProgramStatement,
): void {
  // $FlowExpectedError[prop-missing]
  // $FlowExpectedError[incompatible-use]
  stmt.comments = orgStmt.comments;
  // $FlowExpectedError[incompatible-use]
  stmt.range = orgStmt.range;
  // $FlowExpectedError[incompatible-use]
  stmt.loc = orgStmt.loc;
}

/**
 * Consume an abribray Flow AST and convert it into a Type defintion file.
 *
 * To do this all runtime logic will be stripped and only Type that describe the module boundary will remain.
 */
export default function flowToFlowDef(
  ast: Program,
  code: string,
  scopeManager: ScopeManager,
  opts: TranslationOptions,
): [DetachedNode<Program>, string] {
  const context = createTranslationContext(code, scopeManager, opts);

  const translatedStatements = new Map<
    ProgramStatement,
    DetachedNode<ProgramStatement>,
  >();
  function storeTranslatedStatement(
    stmt: DetachedNode<ProgramStatement>,
    orgStmt: ProgramStatement,
  ): void {
    translatedStatements.set(orgStmt, stmt);
  }

  const seenDeps = new Set<Dep>();
  const processedStatements = new Set<ProgramStatement>();
  const pendingStatements = new Set<ProgramStatement>();
  function updatePendingStatements(deps: TranslatedDeps): void {
    for (const dep of deps) {
      if (seenDeps.has(dep)) {
        continue;
      }
      seenDeps.add(dep);
      const variable = context.variableMap.get(dep);
      if (variable == null) {
        throw new Error(
          `updatePendingStatements: Variable for dependency "${dep}" not found`,
        );
      }
      for (const def of variable.defs) {
        const stmt = def.node;
        if (stmt == null) {
          throw new Error(
            `updatePendingStatements: Variable parent of "${dep}" not found`,
          );
        }
        const topLevelStmt = getTopLevelStatement(stmt, context);
        if (processedStatements.has(topLevelStmt)) {
          continue;
        }
        pendingStatements.add(topLevelStmt);
      }
    }
  }
  function updateProcessedStatement(stmt: ProgramStatement): void {
    processedStatements.add(stmt);
    pendingStatements.delete(stmt);
  }

  // Process all export statements
  for (const stmt of ast.body) {
    const resultExport = convertExport(stmt, context);
    if (resultExport != null) {
      updateProcessedStatement(stmt);
      const [resultExportedStmt, deps] = resultExport;
      storeTranslatedStatement(resultExportedStmt, stmt);
      updatePendingStatements(deps);
    }
  }

  while (pendingStatements.size > 0) {
    const stmt = pendingStatements.values().next().value;
    if (stmt == null) {
      throw new Error(
        `flowToFlowDef: Invalid state, "pendingStatements" cannot be empty`,
      );
    }
    updateProcessedStatement(stmt);
    const [resultStmt, deps] = convertStatement(stmt, context);
    storeTranslatedStatement(resultStmt, stmt);
    updatePendingStatements(deps);
  }

  const translatedBody: Array<DetachedNode<ProgramStatement>> = [];
  for (const stmt of ast.body) {
    const translatedStatement = translatedStatements.get(stmt);
    if (translatedStatement != null) {
      const optimizedStatement = stripUnusedDefs(
        translatedStatement,
        seenDeps,
        context,
      );
      transferProgramStatementProperties(optimizedStatement, stmt);
      translatedBody.push(optimizedStatement);
    }
  }

  return [
    t.Program({
      body: translatedBody,
      sourceType: ast.sourceType,
      interpreter: ast.interpreter?.value ?? null,
      comments: ast.comments,
      tokens: ast.tokens,
      docblock: ast.docblock,
    }),
    code,
  ];
}

function convertExport(
  stmt: ProgramStatement,
  context: TranslationContext,
): ?TranslatedResult<ProgramStatement> {
  switch (stmt.type) {
    case 'ExportNamedDeclaration': {
      return convertExportNamedDeclaration(stmt, context);
    }
    case 'ExportDefaultDeclaration': {
      return convertExportDefaultDeclaration(stmt, context);
    }
    case 'ExportAllDeclaration':
    case 'DeclareExportAllDeclaration': {
      return [asDetachedNode(stmt), []];
    }
    case 'DeclareExportDeclaration':
    case 'DeclareModuleExports': {
      return [asDetachedNode(stmt), analyzeTypeDependencies(stmt, context)];
    }
    default: {
      // Skip non exported functions
      return null;
    }
  }
}

function stripUnusedDefs(
  detachedStmt: DetachedNode<ProgramStatement>,
  usedDeps: Set<Dep>,
  context: TranslationContext,
): DetachedNode<ProgramStatement> {
  // $FlowExpectedError[incompatible-type]
  const stmt: ProgramStatement = detachedStmt;
  switch (stmt.type) {
    case 'ImportDeclaration': {
      const resultSpecfiers = stmt.specifiers.filter(spec =>
        usedDeps.has(spec.local.name),
      );
      if (resultSpecfiers.length === 0) {
        throw translationError(
          stmt,
          `stripUnusedDefs ImportDeclaration: No specifiers remaining`,
          context,
        );
      }
      if (resultSpecfiers.length !== stmt.specifiers.length) {
        return t.ImportDeclaration({
          specifiers: resultSpecfiers,
          importKind: stmt.importKind,
          source: stmt.source,
          assertions: stmt.assertions,
        });
      }
      return detachedStmt;
    }
    default: {
      return detachedStmt;
    }
  }
}

function convertStatement(
  stmt: ProgramStatement,
  context: TranslationContext,
): TranslatedResult<ProgramStatement> {
  switch (stmt.type) {
    case 'FunctionDeclaration': {
      const [result, deps] = convertFunctionDeclation(stmt, context);
      return [result, deps];
    }
    case 'ClassDeclaration': {
      const [result, deps] = convertClassDeclaration(stmt, context);
      return [result, deps];
    }
    case 'InterfaceDeclaration': {
      const [result, deps] = convertInterfaceDeclaration(stmt, context);
      return [result, deps];
    }
    case 'TypeAlias': {
      const [result, deps] = convertTypeAlias(stmt, context);
      return [result, deps];
    }
    case 'OpaqueType': {
      const [result, deps] = convertOpaqueType(stmt, context);
      return [result, deps];
    }
    case 'ImportDeclaration': {
      const [result, deps] = convertImportDeclaration(stmt, context);
      return [result, deps];
    }
    case 'VariableDeclaration': {
      const [result, deps] = convertVariableDeclaration(stmt, context);
      return [result, deps];
    }
    case 'DeclareClass':
    case 'DeclareVariable':
    case 'DeclareFunction':
    case 'DeclareModule':
    case 'DeclareInterface':
    case 'DeclareTypeAlias':
    case 'DeclareOpaqueType':
    case 'EnumDeclaration': {
      return [asDetachedNode(stmt), analyzeTypeDependencies(stmt, context)];
    }
    default: {
      throw translationError(
        stmt,
        `Statement: Unsupported statement type of "${stmt.type}"`,
        context,
      );
    }
  }
}

function convertExpressionToTypeAnnotation(
  expr: Expression,
  context: TranslationContext,
): TranslatedResult<TypeAnnotationType> {
  switch (expr.type) {
    case 'TypeCastExpression': {
      const [resultExpr, deps] = convertTypeCastExpression(expr, context);
      return [resultExpr, deps];
    }
    case 'Identifier': {
      return [
        t.GenericTypeAnnotation({id: t.Identifier({name: expr.name})}),
        analyzeTypeDependencies(expr, context),
      ];
    }
    case 'Literal': {
      const [resultExpr, deps] = convertLiteral(expr, context);
      return [resultExpr, deps];
    }
    case 'ObjectExpression': {
      const [resultExpr, deps] = convertObjectExpression(expr, context);
      return [resultExpr, deps];
    }
    case 'ArrowFunctionExpression':
    case 'FunctionExpression': {
      const [resultExpr, deps] = convertAFunction(expr, context);
      return [resultExpr, deps];
    }
    default: {
      return [
        flowFixMeOrError(
          expr,
          `convertExpressionToTypeAnnotation: Unsupported expression of type "${expr.type}", a type annotation is required.`,
          context,
        ),
        [],
      ];
    }
  }
}

function convertObjectExpression(
  expr: ObjectExpression,
  context: TranslationContext,
): TranslatedResult<TypeAnnotationType> {
  const [resultProperties, deps] = convertArray(expr.properties, prop => {
    switch (prop.type) {
      case 'SpreadElement': {
        const [resultExpr, deps] = convertExpressionToTypeAnnotation(
          prop.argument,
          context,
        );
        return [t.ObjectTypeSpreadProperty({argument: resultExpr}), deps];
      }
      case 'Property': {
        if (
          prop.key.type !== 'Identifier' &&
          prop.key.type !== 'StringLiteral'
        ) {
          throw translationError(
            prop.key,
            `ObjectExpression Property: Unsupported key type of "${prop.key.type}"`,
            context,
          );
        }

        if (prop.method === true) {
          if (
            prop.value.type !== 'ArrowFunctionExpression' &&
            prop.value.type !== 'FunctionExpression'
          ) {
            throw translationError(
              prop.key,
              `ObjectExpression Property: Expected method to have a function value, but got ${prop.value.type}`,
              context,
            );
          }

          const [resultExpr, deps] = convertAFunction(prop.value, context);
          return [
            t.ObjectTypeMethodSignature({
              key: asDetachedNode(prop.key),
              value: resultExpr,
            }),
            deps,
          ];
        }

        if (prop.kind === 'get' || prop.kind === 'set') {
          if (
            prop.value.type !== 'ArrowFunctionExpression' &&
            prop.value.type !== 'FunctionExpression'
          ) {
            throw translationError(
              prop.key,
              `ObjectExpression Property: Expected accessor to have a function value, but got ${prop.value.type}`,
              context,
            );
          }

          const kind = prop.kind;
          const [resultExpr, deps] = convertAFunction(prop.value, context);
          return [
            t.ObjectTypeAccessorSignature({
              key: asDetachedNode(prop.key),
              kind,
              value: resultExpr,
            }),
            deps,
          ];
        }

        const [resultExpr, deps] = convertExpressionToTypeAnnotation(
          prop.value,
          context,
        );

        return [
          t.ObjectTypePropertySignature({
            key: asDetachedNode(prop.key),
            value: resultExpr,
            optional: false,
            variance: null,
          }),
          deps,
        ];
      }
    }
  });
  return [
    t.ObjectTypeAnnotation({
      inexact: false,
      exact: false,
      properties: resultProperties,
      indexers: [],
      callProperties: [],
      internalSlots: [],
    }),
    deps,
  ];
}

function convertLiteral(
  expr: Literal,
  context: TranslationContext,
): TranslatedResult<TypeAnnotationType> {
  switch (expr.literalType) {
    case 'bigint': {
      return [t.BigIntLiteralTypeAnnotation({raw: expr.raw}), []];
    }
    case 'boolean': {
      return [
        t.BooleanLiteralTypeAnnotation({raw: expr.raw, value: expr.value}),
        [],
      ];
    }
    case 'null': {
      return [t.NullLiteralTypeAnnotation({}), []];
    }
    case 'numeric': {
      return [
        t.NumberLiteralTypeAnnotation({raw: expr.raw, value: expr.value}),
        [],
      ];
    }
    case 'string': {
      return [
        t.StringLiteralTypeAnnotation({raw: expr.raw, value: expr.value}),
        [],
      ];
    }
    case 'regexp': {
      return [
        t.GenericTypeAnnotation({id: t.Identifier({name: 'RegExp'})}),
        [],
      ];
    }
    default: {
      throw translationError(
        expr,
        'convertLiteral: Unsupported literal type.',
        context,
      );
    }
  }
}

function convertExportDeclaration(
  decl:
    | ExportDefaultDeclaration['declaration']
    | $NonMaybeType<ExportNamedDeclaration['declaration']>,
  opts: {default: boolean},
  context: TranslationContext,
): TranslatedResult<ProgramStatement> {
  switch (decl.type) {
    case 'FunctionDeclaration': {
      const [declDecl, deps] = convertFunctionDeclation(decl, context);
      return [
        opts.default
          ? t.DeclareExportDefaultDeclaration({
              declaration: declDecl,
            })
          : t.DeclareExportDeclarationNamedWithDeclaration({
              declaration: declDecl,
            }),
        deps,
      ];
    }
    case 'ClassDeclaration': {
      const [declDecl, deps] = convertClassDeclaration(decl, context);
      return [
        opts.default
          ? t.DeclareExportDefaultDeclaration({
              declaration: declDecl,
            })
          : t.DeclareExportDeclarationNamedWithDeclaration({
              declaration: declDecl,
            }),
        deps,
      ];
    }
    case 'InterfaceDeclaration': {
      if (opts.default) {
        throw translationError(
          decl,
          'ExportDeclaration: Default interface found, invalid AST.',
          context,
        );
      }

      const [declDecl, deps] = convertInterfaceDeclaration(decl, context);
      return [
        t.ExportNamedDeclarationWithDeclaration({
          exportKind: 'type',
          declaration: declDecl,
        }),
        deps,
      ];
    }
    case 'TypeAlias': {
      if (opts.default) {
        throw translationError(
          decl,
          'ExportDeclaration: Default type alias found, invalid AST.',
          context,
        );
      }

      const [declDecl, deps] = convertTypeAlias(decl, context);
      return [
        t.ExportNamedDeclarationWithDeclaration({
          exportKind: 'type',
          declaration: declDecl,
        }),
        deps,
      ];
    }
    case 'OpaqueType': {
      if (opts.default) {
        throw translationError(
          decl,
          'ExportDeclaration: Default opaque type found, invalid AST.',
          context,
        );
      }
      const [declDecl, deps] = convertOpaqueType(decl, context);
      return [
        t.DeclareExportDeclarationNamedWithDeclaration({
          declaration: declDecl,
        }),
        deps,
      ];
    }
    case 'VariableDeclaration': {
      if (opts.default) {
        throw translationError(
          decl,
          'ExportDeclaration: Default VariableDeclaration found, invalid AST.',
          context,
        );
      }
      const [declDecl, deps] = convertVariableDeclaration(decl, context);
      return [
        t.DeclareExportDeclarationNamedWithDeclaration({
          declaration: declDecl,
        }),
        deps,
      ];
    }
    case 'EnumDeclaration': {
      return [
        t.ExportNamedDeclarationWithDeclaration({
          exportKind: 'value',
          declaration: asDetachedNode(decl),
        }),
        [],
      ];
    }
    default: {
      if (isExpression(decl)) {
        if (!opts.default) {
          throw translationError(
            decl,
            'ExportDeclaration: Non default expression found, invalid AST.',
            context,
          );
        }
        const [declDecl, deps] = convertExpressionToTypeAnnotation(
          decl,
          context,
        );
        return [
          t.DeclareExportDefaultDeclaration({
            declaration: declDecl,
          }),
          deps,
        ];
      }
      throw translationError(
        decl,
        `ExportDeclaration: Unsupported declaration of type "${decl.type}".`,
        context,
      );
    }
  }
}

function convertExportDefaultDeclaration(
  stmt: ExportDefaultDeclaration,
  context: TranslationContext,
): TranslatedResult<ProgramStatement> {
  return convertExportDeclaration(stmt.declaration, {default: true}, context);
}

function convertExportNamedDeclaration(
  stmt: ExportNamedDeclaration,
  context: TranslationContext,
): TranslatedResult<ProgramStatement> {
  const decl = stmt.declaration;
  if (decl != null) {
    return convertExportDeclaration(decl, {default: false}, context);
  }

  const resultSpecfiers = stmt.specifiers.map(spec => asDetachedNode(spec));
  const specifiersDeps: Array<Dep> =
    stmt.source != null
      ? []
      : stmt.specifiers.flatMap(({local}) =>
          analyzeTypeDependencies(local, context),
        );
  return [
    t.ExportNamedDeclarationWithSpecifiers({
      exportKind: stmt.exportKind,
      source: asDetachedNode(stmt.source),
      specifiers: resultSpecfiers,
    }),
    specifiersDeps,
  ];
}

function convertVariableDeclaration(
  stmt: VariableDeclaration,
  context: TranslationContext,
): TranslatedResult<DeclareVariable> {
  const [first, ...rest] = stmt.declarations;
  if (rest.length > 0) {
    throw translationError(
      stmt,
      `VariableDeclaration: more than one VariableDeclarators found`,
      context,
    );
  }

  const id = first.id;
  if (id.type !== 'Identifier') {
    throw translationError(
      id,
      `VariableDeclaration: unsupported destructing`,
      context,
    );
  }

  const [resultTypeAnnotation, annotDeps] = (() => {
    if (id.typeAnnotation != null) {
      return convertTypeAnnotation(id.typeAnnotation, id, context);
    }

    const init = first.init;
    if (init == null) {
      return [
        flowFixMeOrError(
          first,
          `VariableDeclaration: Type annotation missing`,
          context,
        ),
        [],
      ];
    }

    return convertExpressionToTypeAnnotation(init, context);
  })();

  return [
    t.DeclareVariable({
      id: t.Identifier({
        name: id.name,
        typeAnnotation: t.TypeAnnotation({
          typeAnnotation: resultTypeAnnotation,
        }),
        optional: false,
      }),
    }),
    annotDeps,
  ];
}

function convertImportDeclaration(
  stmt: ImportDeclaration,
  context: TranslationContext,
): TranslatedResult<ImportDeclaration> {
  if (stmt.assertions.length > 0) {
    throw translationError(
      stmt,
      'ImportDeclaration: assertions not supported',
      context,
    );
  }

  return [
    t.ImportDeclaration({
      specifiers: stmt.specifiers,
      importKind: stmt.importKind,
      source: stmt.source,
      assertions: [],
    }),
    [],
  ];
}

function convertInterfaceDeclaration(
  interface_: InterfaceDeclaration,
  context: TranslationContext,
): TranslatedResult<InterfaceDeclaration> {
  return [
    asDetachedNode(interface_),
    analyzeTypeDependencies(interface_, context),
  ];
}

function convertClassDeclaration(
  class_: ClassDeclaration,
  context: TranslationContext,
): TranslatedResult<DeclareClass> {
  const [resultTypeParams, typeParamsDeps] =
    convertTypeParameterDeclarationOrNull(class_.typeParameters, context);
  const implementsDeps = class_.implements.flatMap(impl =>
    analyzeTypeDependencies(impl, context),
  );
  const [resultSuperClass, superClassDeps] = convertSuperClass(
    class_.superClass,
    class_.superTypeParameters,
    context,
  );
  const [resultClassBody, bodyDeps] = convertClassBody(class_.body, context);

  if (class_.decorators.length > 0) {
    throw translationError(
      class_,
      'ClassDeclaration: decorators not supported',
      context,
    );
  }
  return [
    t.DeclareClass({
      id: asDetachedNode(class_.id),
      typeParameters: resultTypeParams,
      implements: class_.implements.map(impl => asDetachedNode(impl)),
      extends: resultSuperClass == null ? [] : [resultSuperClass],
      mixins: [],
      body: resultClassBody,
    }),
    [...typeParamsDeps, ...implementsDeps, ...superClassDeps, ...bodyDeps],
  ];
}

function convertSuperClass(
  superClass: ?Expression,
  superTypeParameters: ?TypeParameterInstantiation,
  context: TranslationContext,
): TranslatedResultOrNull<InterfaceExtends> {
  if (superClass == null) {
    return EMPTY_TRANSLATION_RESULT;
  }

  if (superClass.type !== 'Identifier') {
    throw translationError(
      superClass,
      `SuperClass: Non identifier super type of "${superClass.type}" not supported`,
      context,
    );
  }
  const [resultTypeParams, typeParamsDeps] =
    convertTypeParameterInstantiationOrNull(superTypeParameters, context);
  const superDeps = analyzeTypeDependencies(superClass, context);
  return [
    t.InterfaceExtends({
      id: asDetachedNode(superClass),
      typeParameters: resultTypeParams,
    }),
    [...typeParamsDeps, ...superDeps],
  ];
}

function convertClassBody(
  body: ClassBody,
  context: TranslationContext,
): TranslatedResult<ObjectTypeAnnotation> {
  const [resultProperties, deps] = convertArray(body.body, member =>
    convertClassMember(member, context),
  );

  return [
    t.ObjectTypeAnnotation({
      inexact: false,
      exact: false,
      properties: resultProperties,
      indexers: [],
      callProperties: [],
      internalSlots: [],
    }),
    deps,
  ];
}

function convertClassMember(
  member: ClassMember,
  context: TranslationContext,
): TranslatedResultOrNull<ObjectTypeProperty> {
  switch (member.type) {
    case 'PropertyDefinition': {
      // PrivateIdentifier's are not exposed so can be stripped.
      if (member.key.type === 'PrivateIdentifier') {
        return EMPTY_TRANSLATION_RESULT;
      }
      if (
        member.key.type !== 'Identifier' &&
        member.key.type !== 'StringLiteral'
      ) {
        throw translationError(
          member.key,
          `ClassMember PropertyDefinition: Unsupported key type of "${member.key.type}"`,
          context,
        );
      }

      const [resultTypeAnnotation, deps] = convertTypeAnnotation(
        member.typeAnnotation,
        member,
        context,
      );

      return [
        t.ObjectTypePropertySignature({
          key: asDetachedNode(member.key),
          value: resultTypeAnnotation,
          optional: member.optional,
          static: member.static,
          variance: member.variance,
        }),
        deps,
      ];
    }
    case 'MethodDefinition': {
      // PrivateIdentifier's are not exposed so can be stripped.
      if (member.key.type === 'PrivateIdentifier') {
        return EMPTY_TRANSLATION_RESULT;
      }
      if (
        member.key.type !== 'Identifier' &&
        member.key.type !== 'StringLiteral'
      ) {
        throw translationError(
          member.key,
          `ClassMember PropertyDefinition: Unsupported key type of "${member.key.type}"`,
          context,
        );
      }

      const [resultValue, deps] = convertAFunction(member.value, context);

      if (member.kind === 'get' || member.kind === 'set') {
        // accessors are methods - but flow accessor signatures are properties
        const kind = member.kind;
        return [
          t.ObjectTypeAccessorSignature({
            key: asDetachedNode(member.key),
            value: resultValue,
            static: member.static,
            kind,
          }),
          deps,
        ];
      }

      return [
        t.ObjectTypeMethodSignature({
          key: asDetachedNode(member.key),
          value: resultValue,
          static: member.static,
        }),
        deps,
      ];
    }
    default: {
      throw translationError(
        member,
        `ClassMember: Unsupported member type of "${member.type}"`,
        context,
      );
    }
  }
}

function convertFunctionDeclation(
  func: FunctionDeclaration,
  context: TranslationContext,
): TranslatedResult<DeclareFunction> {
  const id = func.id;
  if (id == null) {
    throw translationError(func, `FunctionDeclaration: Missing name`, context);
  }

  const [resultFunc, funcDeps] = convertAFunction(func, context);

  const [resultPredicate, predicateDeps] = (() => {
    if (func.predicate == null) {
      return EMPTY_TRANSLATION_RESULT;
    }
    const body = func.body.body;
    const predicateExpr =
      body.length === 1 &&
      body[0].type === 'ReturnStatement' &&
      body[0].argument != null
        ? body[0].argument
        : null;
    if (predicateExpr == null) {
      throw translationError(
        func,
        'FunctionDeclation: Invalid predicate function.',
        context,
      );
    }
    return [
      t.DeclaredPredicate({value: asDetachedNode(predicateExpr)}),
      analyzeTypeDependencies(predicateExpr, context),
    ];
  })();

  return [
    t.DeclareFunction({
      name: id.name,
      functionType: resultFunc,
      predicate: resultPredicate,
    }),
    [...funcDeps, ...predicateDeps],
  ];
}

function convertAFunction(
  func: AFunction,
  context: TranslationContext,
): TranslatedResult<FunctionTypeAnnotation> {
  const returnType = analyzeFunctionReturn(func);
  const [resultReturnType, returnDeps] = convertTypeAnnotation(
    returnType,
    func,
    context,
  );

  const [resultParams, restParam, paramsDeps] = convertFunctionParameters(
    func.params,
    context,
  );

  const [resultTypeParams, typeParamsDeps] =
    convertTypeParameterDeclarationOrNull(func.typeParameters, context);

  return [
    t.FunctionTypeAnnotation({
      params: resultParams,
      returnType: resultReturnType,
      rest: restParam,
      typeParameters: resultTypeParams,
    }),
    [...paramsDeps, ...returnDeps, ...typeParamsDeps],
  ];
}

type TranslatedFunctionParametersResults = [
  $ReadOnlyArray<DetachedNode<FunctionTypeParam>>,
  ?DetachedNode<FunctionTypeParam>,
  TranslatedDeps,
];
function convertFunctionParameters(
  params: $ReadOnlyArray<FunctionParameter>,
  context: TranslationContext,
): TranslatedFunctionParametersResults {
  return params.reduce<TranslatedFunctionParametersResults>(
    ([resultParams, restParam, paramsDeps], param) => {
      switch (param.type) {
        case 'Identifier':
        case 'ArrayPattern':
        case 'ObjectPattern': {
          const [resultParam, deps] = convertBindingNameToFunctionTypeParam(
            param,
            context,
          );
          return [
            [...resultParams, resultParam],
            restParam,
            [...paramsDeps, ...deps],
          ];
        }
        case 'AssignmentPattern': {
          const [resultParam, deps] = convertBindingNameToFunctionTypeParam(
            param.left,
            context,
          );
          return [
            [...resultParams, resultParam],
            restParam,
            [...paramsDeps, ...deps],
          ];
        }
        case 'RestElement': {
          if (restParam != null) {
            throw translationError(
              param,
              `FunctionParameter: Multiple rest elements found`,
              context,
            );
          }
          const [resultParam, deps] = convertBindingNameToFunctionTypeParam(
            // $FlowFixMe[incompatible-call] I dont think these other cases are possible
            param.argument,
            context,
          );
          return [resultParams, resultParam, [...paramsDeps, ...deps]];
        }
      }
    },
    [[], null, []],
  );
}

function convertBindingNameToFunctionTypeParam(
  pat: BindingName,
  context: TranslationContext,
): TranslatedResult<FunctionTypeParam> {
  const name = pat.type === 'Identifier' ? pat.name : null;
  const [resultParamTypeAnnotation, paramDeps] = convertTypeAnnotation(
    pat.typeAnnotation,
    pat,
    context,
  );
  return [
    t.FunctionTypeParam({
      name: name != null ? t.Identifier({name}) : null,
      typeAnnotation: resultParamTypeAnnotation,
      optional: pat.type === 'Identifier' ? pat.optional : false,
    }),
    paramDeps,
  ];
}

function convertTypeAlias(
  typeAlias: TypeAlias,
  context: TranslationContext,
): TranslatedResult<TypeAlias> {
  const [typeAnnotationType, rightDeps] = convertTypeAnnotationType(
    typeAlias.right,
    typeAlias,
    context,
  );
  const [typeParameters, typeParamsDeps] =
    convertTypeParameterDeclarationOrNull(typeAlias.typeParameters, context);
  return [
    t.TypeAlias({
      right: typeAnnotationType,
      id: asDetachedNode(typeAlias.id),
      typeParameters,
    }),
    [...rightDeps, ...typeParamsDeps],
  ];
}

function convertOpaqueType(
  opaqueType: OpaqueType,
  context: TranslationContext,
): TranslatedResult<DeclareOpaqueType> {
  const [resultSupertype, supertypeDeps] = convertTypeAnnotationTypeOrNull(
    opaqueType.supertype,
    context,
  );
  const [typeParameters, typeParamsDeps] =
    convertTypeParameterDeclarationOrNull(opaqueType.typeParameters, context);
  return [
    t.DeclareOpaqueType({
      id: asDetachedNode(opaqueType.id),
      typeParameters,
      supertype: resultSupertype,
    }),
    [...typeParamsDeps, ...supertypeDeps],
  ];
}

function convertTypeCastExpression(
  typeCast: TypeCastExpression,
  context: TranslationContext,
): TranslatedResult<TypeAnnotationType> {
  return convertTypeAnnotation(typeCast.typeAnnotation, typeCast, context);
}

function convertTypeAnnotation(
  annot: ?TypeAnnotation,
  container: ESNode,
  context: TranslationContext,
): TranslatedResult<TypeAnnotationType> {
  return convertTypeAnnotationType(annot?.typeAnnotation, container, context);
}
function convertTypeAnnotationType(
  annot: ?TypeAnnotationType,
  container: ESNode,
  context: TranslationContext,
): TranslatedResult<TypeAnnotationType> {
  if (annot == null) {
    return [
      flowFixMeOrError(
        container,
        `TypeAnnotationType: Type annotation missing`,
        context,
      ),
      [],
    ];
  }

  return [asDetachedNode(annot), analyzeTypeDependencies(annot, context)];
}
function convertTypeAnnotationTypeOrNull(
  annot: ?TypeAnnotationType,
  context: TranslationContext,
): TranslatedResultOrNull<TypeAnnotationType> {
  if (annot == null) {
    return EMPTY_TRANSLATION_RESULT;
  }

  return [asDetachedNode(annot), analyzeTypeDependencies(annot, context)];
}
function convertTypeParameterDeclarationOrNull(
  decl: ?TypeParameterDeclaration,
  context: TranslationContext,
): TranslatedResultOrNull<TypeParameterDeclaration> {
  if (decl == null) {
    return EMPTY_TRANSLATION_RESULT;
  }
  return [asDetachedNode(decl), analyzeTypeDependencies(decl, context)];
}
function convertTypeParameterInstantiationOrNull(
  inst: ?TypeParameterInstantiation,
  context: TranslationContext,
): TranslatedResultOrNull<TypeParameterInstantiation> {
  if (inst == null) {
    return EMPTY_TRANSLATION_RESULT;
  }
  return [asDetachedNode(inst), analyzeTypeDependencies(inst, context)];
}
