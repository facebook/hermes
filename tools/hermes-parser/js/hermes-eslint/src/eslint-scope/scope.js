/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {
  ArrowFunctionExpression,
  BlockStatement,
  CatchClause,
  ClassDeclaration,
  ClassExpression,
  DeclareClass,
  DeclareInterface,
  DeclareModule,
  DeclareOpaqueType,
  DeclareTypeAlias,
  ESNode as Node,
  ForInStatement,
  ForOfStatement,
  ForStatement,
  FunctionDeclaration,
  FunctionExpression,
  FunctionTypeAnnotation,
  Identifier,
  InterfaceDeclaration,
  OpaqueType,
  Program,
  SwitchStatement,
  TypeAlias,
  WithStatement,
} from 'hermes-estree';
import type {Definition} from './definition';
import type {ImplicitGlobal, ReadWriteFlagType} from './reference';
import type ScopeManager from './scope-manager';

const {ReadWriteFlag, Reference} = require('./reference');
const Variable = require('./variable');
const {
  DefinitionType,
  FunctionNameDefinition,
  ImplicitGlobalVariableDefinition,
} = require('./definition');
const assert = require('assert');

const ScopeType = ({
  Block: 'block',
  Catch: 'catch',
  Class: 'class',
  DeclareModule: 'declare-module',
  For: 'for',
  Function: 'function',
  FunctionExpressionName: 'function-expression-name',
  Global: 'global',
  Module: 'module',
  Switch: 'switch',
  Type: 'type',
  With: 'with',
}: $ReadOnly<{
  Block: 'block',
  Catch: 'catch',
  Class: 'class',
  DeclareModule: 'declare-module',
  For: 'for',
  Function: 'function',
  FunctionExpressionName: 'function-expression-name',
  Global: 'global',
  Module: 'module',
  Switch: 'switch',
  Type: 'type',
  With: 'with',
}>);

/**
 * Test if scope is strict
 */
function isStrictScope(scope: Scope, isMethodDefinition: boolean): boolean {
  // When upper scope exists and is strict, inner scope is also strict.
  if (scope.upper && scope.upper.isStrict) {
    return true;
  }

  if (isMethodDefinition) {
    return true;
  }

  if (scope.type === ScopeType.Class || scope.type === ScopeType.Module) {
    return true;
  }

  if (scope.type === ScopeType.Block || scope.type === ScopeType.Switch) {
    return false;
  }

  const body = (() => {
    if (scope.type === ScopeType.Function) {
      if (scope.block.type === 'ArrowFunctionExpression') {
        if (scope.block.body.type !== 'BlockStatement') {
          return false;
        } else {
          return scope.block.body;
        }
      } else if (scope.block.type === 'Program') {
        return scope.block;
      } else {
        return scope.block.body;
      }
    } else if (scope.type === ScopeType.Global) {
      return scope.block;
    } else {
      return false;
    }
  })();
  if (body === false) {
    return false;
  }

  // Search 'use strict' directive.
  for (let i = 0, iz = body.body.length; i < iz; ++i) {
    const stmt = body.body[i];

    if (stmt.type !== 'ExpressionStatement') {
      break;
    }
    const expr = stmt.expression;

    if (expr.type !== 'Literal' || typeof expr.value !== 'string') {
      break;
    }
    if (expr.raw !== null && expr.raw !== undefined) {
      if (expr.raw === '"use strict"' || expr.raw === "'use strict'") {
        return true;
      }
    } else {
      if (expr.value === 'use strict') {
        return true;
      }
    }
  }

  return false;
}

/**
 * Register scope
 */
function registerScope(scopeManager: ScopeManager, scope: Scope): void {
  scopeManager.scopes.push(scope);

  const scopes = scopeManager.__nodeToScope.get(scope.block);

  if (scopes) {
    scopes.push(scope);
  } else {
    scopeManager.__nodeToScope.set(scope.block, [scope]);
  }
}

/**
 * Should be statically closed
 */
function shouldBeStaticallyClosed(def: Definition): boolean {
  return (
    def.type === DefinitionType.ClassName ||
    def.type === DefinitionType.Enum ||
    def.type === DefinitionType.Type ||
    (def.type === DefinitionType.Variable &&
      def.parent.type !== 'DeclareVariable' &&
      def.parent.kind !== 'var')
  );
}

function asScope(scope: ScopeBase): Scope {
  // $FlowExpectedError[incompatible-return] - it's impossible to tell flow this is safe
  return scope;
}

/**
 * @class Scope
 */
class ScopeBase {
  /**
   * The type of the scope.
   */
  +type: string;

  /**
   * The scoped {@link Variable}s of this scope, as <code>{ Variable.name
   * : Variable }</code>.
   */
  set: Map<string, Variable> = new Map();

  /**
   * Generally, through the lexical scoping of JS you can always know
   * which variable an identifier in the source code refers to. There are
   * a few exceptions to this rule. With 'global' and 'with' scopes you
   * can only decide at runtime which variable a reference refers to.
   * Moreover, if 'eval()' is used in a scope, it might introduce new
   * bindings in this or its parent scopes.
   * All those scopes are considered 'dynamic'.
   */
  dynamic: boolean;

  /**
   * A reference to the scope-defining 'node'.
   */
  +block: Node;

  /**
   * The {@link Reference|references} that are not resolved with this scope.
   */
  through: Array<Reference> = [];

  /**
   * The scoped {@link Variable}s of this scope. In the case of a
   * 'function' scope this includes the automatic argument <em>arguments</em> as
   * its first element, as well as all further formal arguments.
   */
  variables: Array<Variable> = [];

  /**
   * Any variable {@link Reference|reference} found in this scope. This
   * includes occurrences of local variables as well as variables from
   * parent scopes (including the global scope). For local variables
   * this also includes defining occurrences (like in a 'var' statement).
   * In a 'function' scope this does not include the occurrences of the
   * formal parameter in the parameter list.
   */
  references: Array<Reference> = [];

  /**
   * For 'global' and 'function' scopes, this is a self-reference. For
   * other scope types this is the <em>variableScope</em> value of the
   * parent scope.
   */
  variableScope: Scope;

  /**
   * Whether this scope is created by a FunctionExpression.
   */
  functionExpressionScope: boolean = false;

  thisFound: boolean = false;

  /**
   * List of {@link Reference}s that are left to be resolved (i.e. which
   * need to be linked to the variable they refer to).
   */
  __referencesLeftToResolve: Array<Reference> = [];

  /**
   * Reference to the parent {@link Scope|scope}.
   */
  upper: ?Scope;

  /**
   * Whether 'use strict' is in effect in this scope.
   */
  isStrict: boolean;

  /**
   * List of nested {@link Scope}s.
   */
  childScopes: Array<Scope> = [];

  __declaredVariables: WeakMap<Node, Array<Variable>>;
  __isClosed: boolean = false;

  constructor(
    scopeManager: ScopeManager,
    type: string,
    upperScope: ?Scope,
    block: Node,
    isMethodDefinition: boolean,
  ) {
    this.type = type;
    this.set = new Map();

    this.dynamic =
      this.type === ScopeType.Global || this.type === ScopeType.With;

    this.block = block;

    this.variableScope =
      this.type === ScopeType.Global ||
      this.type === ScopeType.Function ||
      this.type === ScopeType.Module ||
      this.type === ScopeType.DeclareModule
        ? asScope(this)
        : // $FlowFixMe[incompatible-use] upperScope can only be null for Global scope
          upperScope.variableScope;

    this.upper = upperScope;

    this.isStrict = isStrictScope(asScope(this), isMethodDefinition);

    if (this.upper) {
      this.upper.childScopes.push(asScope(this));
    }

    this.__declaredVariables = scopeManager.__declaredVariables;

    registerScope(scopeManager, asScope(this));
  }

  __shouldStaticallyClose(_scopeManager: ScopeManager): boolean {
    return !this.dynamic;
  }

  __shouldStaticallyCloseForGlobal(ref: Reference): boolean {
    // On global scope, let/const/class declarations should be resolved statically.
    const name = ref.identifier.name;

    const variable = this.set.get(name);
    if (variable == null) {
      return false;
    }

    const defs = variable.defs;

    return defs.length > 0 && defs.every(shouldBeStaticallyClosed);
  }

  __staticCloseRef(ref: Reference): void {
    if (!this.__resolve(ref)) {
      this.__delegateToUpperScope(ref);
    }
  }

  __dynamicCloseRef(ref: Reference): void {
    // notify all names are through to global
    let current = this;

    do {
      current.through.push(ref);
      current = current.upper;
    } while (current);
  }

  __globalCloseRef(ref: Reference): void {
    // let/const/class declarations should be resolved statically.
    // others should be resolved dynamically.
    if (this.__shouldStaticallyCloseForGlobal(ref)) {
      this.__staticCloseRef(ref);
    } else {
      this.__dynamicCloseRef(ref);
    }
  }

  __close(scopeManager: ScopeManager): ?Scope {
    let closeRef;

    if (this.__shouldStaticallyClose(scopeManager)) {
      closeRef = ref => this.__staticCloseRef(ref);
    } else if (this.type !== ScopeType.Global) {
      closeRef = ref => this.__dynamicCloseRef(ref);
    } else {
      closeRef = ref => this.__globalCloseRef(ref);
    }

    // Try Resolving all references in this scope.
    for (let i = 0, iz = this.__referencesLeftToResolve.length; i < iz; ++i) {
      const ref = this.__referencesLeftToResolve[i];

      closeRef(ref);
    }
    this.__referencesLeftToResolve = [];
    this.__isClosed = true;

    return this.upper;
  }

  /**
   * Whether a given reference can be resolved to a given variable.
   * May be overridden in scope implementations.
   */
  __isValidResolution(_ref: Reference, _variable: Variable): boolean {
    return true;
  }

  __resolve(ref: Reference): boolean {
    const name = ref.identifier.name;

    const variable = this.set.get(name);
    if (variable == null) {
      return false;
    }

    if (!this.__isValidResolution(ref, variable)) {
      return false;
    }
    variable.references.push(ref);
    ref.resolved = variable;

    return true;
  }

  __delegateToUpperScope(ref: Reference): void {
    if (this.upper) {
      this.upper.__referencesLeftToResolve.push(ref);
    }
    this.through.push(ref);
  }

  __addDeclaredVariablesOfNode(variable: Variable, node: ?Node): void {
    if (node == null) {
      return;
    }

    let variables = this.__declaredVariables.get(node);

    if (variables == null) {
      variables = [];
      this.__declaredVariables.set(node, variables);
    }
    if (variables.indexOf(variable) === -1) {
      variables.push(variable);
    }
  }

  __defineGeneric(
    name: string,
    set: Map<string, Variable>,
    variables: Array<Variable>,
    node: ?Identifier,
    def: ?Definition,
  ): void {
    let variable;

    const existingVariable = set.get(name);
    if (!existingVariable) {
      const newVariable = new Variable(name, asScope(this));
      set.set(name, newVariable);
      variables.push(newVariable);
      variable = newVariable;
    } else {
      variable = existingVariable;
    }

    if (def) {
      variable.defs.push(def);
      this.__addDeclaredVariablesOfNode(variable, def.node);
      this.__addDeclaredVariablesOfNode(variable, def.parent);
    }
    if (node) {
      variable.identifiers.push(node);
    }
  }

  __define(node: ?Node, def: Definition): void {
    if (node && node.type === 'Identifier') {
      this.__defineGeneric(node.name, this.set, this.variables, node, def);
    }
  }

  __referencingValue(
    node: Identifier,
    assign?: ?ReadWriteFlagType,
    writeExpr?: ?Node,
    maybeImplicitGlobal?: ?ImplicitGlobal,
    init?: ?boolean,
  ): void {
    // because Array element may be null
    if (!node || node.type !== 'Identifier') {
      return;
    }

    // Specially handle like `this`.
    if (node.name === 'super') {
      return;
    }

    const ref = new Reference(
      node /* identifier */,
      asScope(this) /* scope */,
      assign || ReadWriteFlag.READ /* read-write flag */,
      writeExpr /* writeExpr */,
      maybeImplicitGlobal /* maybeImplicitGlobal */,
      !!init /* init */,
      false /* isTypeReference */,
    );

    this.references.push(ref);
    this.__referencesLeftToResolve.push(ref);
  }

  __referencingType(node: Identifier): void {
    const ref = new Reference(
      node /* identifier */,
      asScope(this) /* scope */,
      ReadWriteFlag.READ /* read-write flag */,
      null /* writeExpr */,
      null /* maybeImplicitGlobal */,
      false /* init */,
      true /* isTypeReference */,
    );

    this.references.push(ref);
    this.__referencesLeftToResolve.push(ref);
  }

  __detectThis(): void {
    this.thisFound = true;
  }

  /**
   * returns resolved {Reference}
   * @method Scope#resolve
   * @param {Espree.Identifier} ident - identifier to be resolved.
   * @returns {Reference} reference
   */
  resolve(ident: Identifier): ?Reference {
    let ref, i, iz;

    assert(this.__isClosed, 'Scope should be closed.');
    assert(ident.type === 'Identifier', 'Target should be identifier.');
    for (i = 0, iz = this.references.length; i < iz; ++i) {
      ref = this.references[i];
      if (ref.identifier === ident) {
        return ref;
      }
    }
    return null;
  }

  /**
   * returns this scope is static
   */
  isStatic(): boolean {
    return !this.dynamic;
  }

  /**
   * returns this scope has materialized arguments
   */
  isArgumentsMaterialized(): boolean {
    return true;
  }

  /**
   * returns this scope has materialized `this` reference
   */
  isThisMaterialized(): boolean {
    return true;
  }

  isUsedName(name: string): boolean {
    if (this.set.has(name)) {
      return true;
    }
    for (let i = 0, iz = this.through.length; i < iz; ++i) {
      if (this.through[i].identifier.name === name) {
        return true;
      }
    }
    return false;
  }
}

export type Scope =
  | GlobalScope
  | ModuleScope
  | FunctionExpressionNameScope
  | CatchScope
  | WithScope
  | BlockScope
  | SwitchScope
  | FunctionScope
  | ForScope
  | ClassScope
  | TypeScope
  | DeclareModuleScope;

class GlobalScope extends ScopeBase {
  declare +type: (typeof ScopeType)['Global'];
  declare +block: Program;

  implicit: {
    set: Map<string, Variable>,
    variables: Array<Variable>,
    referencesLeftToResolve: Array<Reference>,
  };

  constructor(scopeManager: ScopeManager, block: GlobalScope['block']) {
    super(scopeManager, ScopeType.Global, null, block, false);
    this.implicit = {
      set: new Map(),
      variables: [],
      referencesLeftToResolve: [],
    };
  }

  __close(scopeManager: ScopeManager): ?Scope {
    const implicit = [];

    for (let i = 0, iz = this.__referencesLeftToResolve.length; i < iz; ++i) {
      const ref = this.__referencesLeftToResolve[i];

      if (ref.__maybeImplicitGlobal && !this.set.has(ref.identifier.name)) {
        implicit.push(ref.__maybeImplicitGlobal);
      }
    }

    // create an implicit global variable from assignment expression
    for (let i = 0, iz = implicit.length; i < iz; ++i) {
      const info = implicit[i];

      this.__defineImplicit(
        info.pattern,
        new ImplicitGlobalVariableDefinition(info.pattern, info.node),
      );
    }

    this.implicit.referencesLeftToResolve = this.__referencesLeftToResolve;

    return super.__close(scopeManager);
  }

  __defineImplicit(node: Node, def: Definition): void {
    if (node && node.type === 'Identifier') {
      this.__defineGeneric(
        node.name,
        this.implicit.set,
        this.implicit.variables,
        node,
        def,
      );
    }
  }
}

class ModuleScope extends ScopeBase {
  declare +type: (typeof ScopeType)['Module'];
  declare +block: Program;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: ModuleScope['block'],
  ) {
    super(scopeManager, ScopeType.Module, upperScope, block, false);
  }
}

class FunctionExpressionNameScope extends ScopeBase {
  declare +type: (typeof ScopeType)['FunctionExpressionName'];
  declare +block: FunctionExpression;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: FunctionExpressionNameScope['block'],
  ) {
    super(
      scopeManager,
      ScopeType.FunctionExpressionName,
      upperScope,
      block,
      false,
    );
    assert(block.id);
    this.__define(
      block.id,
      new FunctionNameDefinition(
        // $FlowExpectedError[incompatible-call]
        block.id,
        block,
      ),
    );
    this.functionExpressionScope = true;
  }
}

class CatchScope extends ScopeBase {
  declare +type: (typeof ScopeType)['Catch'];
  declare +block: CatchClause;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: CatchScope['block'],
  ) {
    super(scopeManager, ScopeType.Catch, upperScope, block, false);
  }
}

class WithScope extends ScopeBase {
  declare +type: (typeof ScopeType)['With'];
  declare +block: WithStatement;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: WithScope['block'],
  ) {
    super(scopeManager, ScopeType.With, upperScope, block, false);
  }

  __close(scopeManager: ScopeManager): ?Scope {
    if (this.__shouldStaticallyClose(scopeManager)) {
      return super.__close(scopeManager);
    }

    for (let i = 0, iz = this.__referencesLeftToResolve.length; i < iz; ++i) {
      const ref = this.__referencesLeftToResolve[i];

      this.__delegateToUpperScope(ref);
    }
    this.__referencesLeftToResolve = [];
    this.__isClosed = true;

    return this.upper;
  }
}

class BlockScope extends ScopeBase {
  declare +type: (typeof ScopeType)['Block'];
  declare +block: BlockStatement;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: BlockScope['block'],
  ) {
    super(scopeManager, ScopeType.Block, upperScope, block, false);
  }
}

class SwitchScope extends ScopeBase {
  declare +type: (typeof ScopeType)['Switch'];
  declare +block: SwitchStatement;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: SwitchScope['block'],
  ) {
    super(scopeManager, ScopeType.Switch, upperScope, block, false);
  }
}

class FunctionScope extends ScopeBase {
  declare +type: (typeof ScopeType)['Function'];
  declare +block:
    | ArrowFunctionExpression
    | FunctionDeclaration
    | FunctionExpression
    | Program;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: FunctionScope['block'],
    isMethodDefinition: boolean,
  ) {
    super(
      scopeManager,
      ScopeType.Function,
      upperScope,
      block,
      isMethodDefinition,
    );

    // section 9.2.13, FunctionDeclarationInstantiation.
    // NOTE Arrow functions never have an arguments objects.
    if (this.block.type !== 'ArrowFunctionExpression') {
      this.__defineArguments();
    }
  }

  isArgumentsMaterialized(): boolean {
    // TODO(Constellation)
    // We can more aggressive on this condition like this.
    //
    // function t() {
    //     // arguments of t is always hidden.
    //     function arguments() {
    //     }
    // }
    if (this.block.type === 'ArrowFunctionExpression') {
      return false;
    }

    if (!this.isStatic()) {
      return true;
    }

    const variable = this.set.get('arguments');

    assert(variable, 'Always have arguments variable.');
    return variable?.references.length !== 0;
  }

  isThisMaterialized(): boolean {
    if (!this.isStatic()) {
      return true;
    }
    return this.thisFound;
  }

  __defineArguments(): void {
    this.__defineGeneric('arguments', this.set, this.variables, null, null);
  }

  // References in default parameters isn't resolved to variables which are in their function body.
  //     const x = 1
  //     function f(a = x) { // This `x` is resolved to the `x` in the outer scope.
  //         const x = 2
  //         console.log(a)
  //     }
  __isValidResolution(ref: Reference, variable: Variable): boolean {
    if (!super.__isValidResolution(ref, variable)) {
      return false;
    }

    // If `options.nodejsScope` is true, `this.block` becomes a Program node.
    if (this.block.type === 'Program') {
      return true;
    }

    const bodyStart = this.block.body.range[0];

    // It's invalid resolution in the following case:
    return !(
      (
        variable.scope === this &&
        ref.identifier.range[0] < bodyStart && // the reference is in the parameter part.
        variable.defs.every(d => d.name.range[0] >= bodyStart)
      ) // the variable is in the body.
    );
  }
}

class ForScope extends ScopeBase {
  declare +type: (typeof ScopeType)['For'];
  declare +block: ForInStatement | ForOfStatement | ForStatement;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: ForScope['block'],
  ) {
    super(scopeManager, ScopeType.For, upperScope, block, false);
  }
}

class ClassScope extends ScopeBase {
  declare +type: (typeof ScopeType)['Class'];
  declare +block: ClassDeclaration | ClassExpression;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    block: ClassScope['block'],
  ) {
    super(scopeManager, ScopeType.Class, upperScope, block, false);
  }
}

class TypeScope extends ScopeBase {
  declare +type: (typeof ScopeType)['Type'];
  declare +block:
    | FunctionDeclaration
    | FunctionExpression
    | ArrowFunctionExpression
    | DeclareTypeAlias
    | DeclareOpaqueType
    | DeclareInterface
    | DeclareClass
    | FunctionTypeAnnotation
    | TypeAlias
    | OpaqueType
    | InterfaceDeclaration;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    typeNode: TypeScope['block'],
  ) {
    super(scopeManager, ScopeType.Type, upperScope, typeNode, false);
  }
}

class DeclareModuleScope extends ScopeBase {
  declare +type: (typeof ScopeType)['DeclareModule'];
  declare +block: DeclareModule;

  constructor(
    scopeManager: ScopeManager,
    upperScope: ?Scope,
    declareModuleNode: DeclareModuleScope['block'],
  ) {
    super(
      scopeManager,
      ScopeType.DeclareModule,
      upperScope,
      declareModuleNode,
      false,
    );
  }
}

module.exports = {
  ScopeType,
  GlobalScope,
  ModuleScope,
  FunctionExpressionNameScope,
  CatchScope,
  WithScope,
  BlockScope,
  SwitchScope,
  FunctionScope,
  ForScope,
  ClassScope,
  TypeScope,
  DeclareModuleScope,
};
