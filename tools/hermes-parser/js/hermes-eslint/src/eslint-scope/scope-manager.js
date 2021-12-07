/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {ESNode as Node} from 'hermes-estree';
import type {VisitorKeys} from '../HermesESLintVisitorKeys';
import type {Scope} from './scope';
import type Variable from './variable';

const {
  BlockScope,
  CatchScope,
  ClassScope,
  DeclareModuleScope,
  ForScope,
  FunctionExpressionNameScope,
  FunctionScope,
  GlobalScope,
  ModuleScope,
  ScopeType,
  SwitchScope,
  TypeScope,
  WithScope,
} = require('./scope');
const assert = require('assert');

type ScopeManagerOptions = {
  sourceType: 'module' | 'script',
  childVisitorKeys: VisitorKeys,
};

/**
 * @class ScopeManager
 */
class ScopeManager {
  scopes: Array<Scope> = [];
  globalScope: ?GlobalScope = null;

  __nodeToScope: WeakMap<Node, Array<Scope>> = new WeakMap();
  __currentScope: ?Scope = null;
  __options: ScopeManagerOptions;
  __declaredVariables: WeakMap<Node, Array<Variable>> = new WeakMap();

  constructor(options: ScopeManagerOptions) {
    this.__options = options;
  }

  isModule(): boolean {
    return this.__options.sourceType === 'module';
  }

  // Returns appropriate scope for this node.
  __get(node: Node): ?Array<Scope> {
    return this.__nodeToScope.get(node);
  }

  /**
   * Get variables that are declared by the node.
   *
   * "are declared by the node" means the node is same as `Variable.defs[].node` or `Variable.defs[].parent`.
   * If the node declares nothing, this method returns an empty array.
   * CAUTION: This API is experimental. See https://github.com/estools/escope/pull/69 for more details.
   *
   * @param {Espree.Node} node - a node to get.
   * @returns {Variable[]} variables that declared by the node.
   */
  getDeclaredVariables(node: Node): Array<Variable> {
    return this.__declaredVariables.get(node) || [];
  }

  /**
   * acquire scope from node.
   * @method ScopeManager#acquire
   * @param {Espree.Node} node - node for the acquired scope.
   * @param {boolean=} inner - look up the most inner scope, default value is false.
   * @returns {Scope?} Scope from node
   */
  acquire(node: Node, inner?: boolean): ?Scope {
    /**
     * predicate
     * @param {Scope} testScope - scope to test
     * @returns {boolean} predicate
     */
    function predicate(testScope) {
      if (
        testScope.type === ScopeType.Function &&
        testScope.functionExpressionScope
      ) {
        return false;
      }
      return true;
    }

    const scopes = this.__get(node);

    if (!scopes || scopes.length === 0) {
      return null;
    }

    // Heuristic selection from all scopes.
    // If you would like to get all scopes, please use ScopeManager#acquireAll.
    if (scopes.length === 1) {
      return scopes[0];
    }

    if (inner === true) {
      for (let i = scopes.length - 1; i >= 0; --i) {
        const scope = scopes[i];

        if (predicate(scope)) {
          return scope;
        }
      }
    } else {
      for (let i = 0, iz = scopes.length; i < iz; ++i) {
        const scope = scopes[i];

        if (predicate(scope)) {
          return scope;
        }
      }
    }

    return null;
  }

  /**
   * acquire all scopes from node.
   * @method ScopeManager#acquireAll
   * @param {Espree.Node} node - node for the acquired scope.
   * @returns {Scopes?} Scope array
   */
  acquireAll(node: Node): ?Array<Scope> {
    return this.__get(node);
  }

  /**
   * release the node.
   * @method ScopeManager#release
   * @param {Espree.Node} node - releasing node.
   * @param {boolean=} inner - look up the most inner scope, default value is false.
   * @returns {Scope?} upper scope for the node.
   */
  release(node: Node, inner?: boolean): ?Scope {
    const scopes = this.__get(node);

    if (scopes && scopes.length) {
      const scope = scopes[0].upper;

      if (!scope) {
        return null;
      }
      return this.acquire(scope.block, inner);
    }
    return null;
  }

  attach(): void {}

  detach(): void {}

  __nestScope<S: Scope>(scope: S): S {
    if (scope instanceof GlobalScope) {
      assert(this.__currentScope === null);
      this.globalScope = scope;
    }
    this.__currentScope = scope;
    return scope;
  }

  __nestGlobalScope(node: GlobalScope['block']): GlobalScope {
    return this.__nestScope(new GlobalScope(this, node));
  }

  __nestBlockScope(node: BlockScope['block']): BlockScope {
    return this.__nestScope(new BlockScope(this, this.__currentScope, node));
  }

  __nestFunctionScope(
    node: FunctionScope['block'],
    isMethodDefinition: boolean,
  ): FunctionScope {
    return this.__nestScope(
      new FunctionScope(this, this.__currentScope, node, isMethodDefinition),
    );
  }

  __nestForScope(node: ForScope['block']): ForScope {
    return this.__nestScope(new ForScope(this, this.__currentScope, node));
  }

  __nestCatchScope(node: CatchScope['block']): CatchScope {
    return this.__nestScope(new CatchScope(this, this.__currentScope, node));
  }

  __nestWithScope(node: WithScope['block']): WithScope {
    return this.__nestScope(new WithScope(this, this.__currentScope, node));
  }

  __nestClassScope(node: ClassScope['block']): ClassScope {
    return this.__nestScope(new ClassScope(this, this.__currentScope, node));
  }

  __nestSwitchScope(node: SwitchScope['block']): SwitchScope {
    return this.__nestScope(new SwitchScope(this, this.__currentScope, node));
  }

  __nestModuleScope(node: ModuleScope['block']): ModuleScope {
    return this.__nestScope(new ModuleScope(this, this.__currentScope, node));
  }

  __nestTypeScope(node: TypeScope['block']): TypeScope {
    return this.__nestScope(new TypeScope(this, this.__currentScope, node));
  }

  __nestDeclareModuleScope(
    node: DeclareModuleScope['block'],
  ): DeclareModuleScope {
    return this.__nestScope(
      new DeclareModuleScope(this, this.__currentScope, node),
    );
  }

  __nestFunctionExpressionNameScope(
    node: FunctionExpressionNameScope['block'],
  ): FunctionExpressionNameScope {
    return this.__nestScope(
      new FunctionExpressionNameScope(this, this.__currentScope, node),
    );
  }
}

module.exports = ScopeManager;
