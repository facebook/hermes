/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import Renamer from './lib/renamer';
import type NodePath from '../path';
import traverse from '../index';
import Binding from './binding';
import globals from 'globals';
import * as t from '../../types';
import {scope as scopeCache} from '../cache';

// Recursively gathers the identifying names of a node.
function gatherNodeParts(node: Object, parts: Array) {
  switch (node?.type) {
    default:
      if (t.isModuleDeclaration(node)) {
        if (node.source) {
          gatherNodeParts(node.source, parts);
        } else if (node.specifiers && node.specifiers.length) {
          for (const e of node.specifiers) gatherNodeParts(e, parts);
        } else if (node.declaration) {
          gatherNodeParts(node.declaration, parts);
        }
      } else if (t.isModuleSpecifier(node)) {
        gatherNodeParts(node.local, parts);
      } else if (t.isLiteral(node)) {
        parts.push(node.value);
      }
      break;

    case 'MemberExpression':
    case 'OptionalMemberExpression':
    case 'JSXMemberExpression':
      gatherNodeParts(node.object, parts);
      gatherNodeParts(node.property, parts);
      break;

    case 'Identifier':
    case 'JSXIdentifier':
      parts.push(node.name);
      break;

    case 'CallExpression':
    case 'OptionalCallExpression':
    case 'NewExpression':
      gatherNodeParts(node.callee, parts);
      break;

    case 'ObjectExpression':
    case 'ObjectPattern':
      for (const e of node.properties) {
        gatherNodeParts(e, parts);
      }
      break;

    case 'SpreadElement':
    case 'RestElement':
      gatherNodeParts(node.argument, parts);
      break;

    case 'ObjectProperty':
    case 'ObjectMethod':
    case 'ClassProperty':
    case 'ClassMethod':
    case 'ClassPrivateProperty':
    case 'ClassPrivateMethod':
      gatherNodeParts(node.key, parts);
      break;

    case 'ThisExpression':
      parts.push('this');
      break;

    case 'Super':
      parts.push('super');
      break;

    case 'Import':
      parts.push('import');
      break;

    case 'DoExpression':
      parts.push('do');
      break;

    case 'YieldExpression':
      parts.push('yield');
      gatherNodeParts(node.argument, parts);
      break;

    case 'AwaitExpression':
      parts.push('await');
      gatherNodeParts(node.argument, parts);
      break;

    case 'AssignmentExpression':
      gatherNodeParts(node.left, parts);
      break;

    case 'VariableDeclarator':
      gatherNodeParts(node.id, parts);
      break;

    case 'FunctionExpression':
    case 'FunctionDeclaration':
    case 'ClassExpression':
    case 'ClassDeclaration':
      gatherNodeParts(node.id, parts);
      break;

    case 'PrivateName':
      gatherNodeParts(node.id, parts);
      break;

    case 'ParenthesizedExpression':
      gatherNodeParts(node.expression, parts);
      break;

    case 'UnaryExpression':
    case 'UpdateExpression':
      gatherNodeParts(node.argument, parts);
      break;

    case 'MetaProperty':
      gatherNodeParts(node.meta, parts);
      gatherNodeParts(node.property, parts);
      break;

    case 'JSXElement':
      gatherNodeParts(node.openingElement, parts);
      break;

    case 'JSXOpeningElement':
      parts.push(node.name);
      break;

    case 'JSXFragment':
      gatherNodeParts(node.openingFragment, parts);
      break;

    case 'JSXOpeningFragment':
      parts.push('Fragment');
      break;

    case 'JSXNamespacedName':
      gatherNodeParts(node.namespace, parts);
      gatherNodeParts(node.name, parts);
      break;
  }
}

//

const collectorVisitor = {
  For(path) {
    for (const key of (t.FOR_INIT_KEYS: Array)) {
      const declar = path.get(key);
      // delegate block scope handling to the `BlockScoped` method
      if (declar.isVar()) {
        const parentScope =
          path.scope.getFunctionParent() || path.scope.getProgramParent();
        parentScope.registerBinding('var', declar);
      }
    }
  },

  Declaration(path) {
    // delegate block scope handling to the `BlockScoped` method
    if (path.isBlockScoped()) return;

    // this will be hit again once we traverse into it after this iteration
    if (path.isExportDeclaration() && path.get('declaration').isDeclaration()) {
      return;
    }

    // we've ran into a declaration!
    const parent =
      path.scope.getFunctionParent() || path.scope.getProgramParent();
    parent.registerDeclaration(path);
  },

  ReferencedIdentifier(path, state) {
    state.references.push(path);
  },

  ForXStatement(path, state) {
    const left = path.get('left');
    if (left.isPattern() || left.isIdentifier()) {
      state.constantViolations.push(path);
    }
  },

  ExportDeclaration: {
    exit(path) {
      const {node, scope} = path;
      const declar = node.declaration;
      if (t.isClassDeclaration(declar) || t.isFunctionDeclaration(declar)) {
        const id = declar.id;
        if (!id) return;

        const binding = scope.getBinding(id.name);
        if (binding) binding.reference(path);
      } else if (t.isVariableDeclaration(declar)) {
        for (const decl of (declar.declarations: Array<Object>)) {
          for (const name of Object.keys(t.getBindingIdentifiers(decl))) {
            const binding = scope.getBinding(name);
            if (binding) binding.reference(path);
          }
        }
      }
    },
  },

  LabeledStatement(path) {
    path.scope.getProgramParent().addGlobal(path.node);
    path.scope.getBlockParent().registerDeclaration(path);
  },

  AssignmentExpression(path, state) {
    state.assignments.push(path);
  },

  UpdateExpression(path, state) {
    state.constantViolations.push(path);
  },

  UnaryExpression(path, state) {
    if (path.node.operator === 'delete') {
      state.constantViolations.push(path);
    }
  },

  BlockScoped(path) {
    let scope = path.scope;
    if (scope.path === path) scope = scope.parent;

    const parent = scope.getBlockParent();
    parent.registerDeclaration(path);

    // Register class identifier in class' scope if this is a class declaration.
    if (path.isClassDeclaration() && path.node.id) {
      const id = path.node.id;
      const name = id.name;

      path.scope.bindings[name] = path.scope.parent.getBinding(name);
    }
  },

  Block(path) {
    const paths = path.get('body');
    for (const bodyPath of (paths: Array)) {
      if (bodyPath.isFunctionDeclaration()) {
        path.scope.getBlockParent().registerDeclaration(bodyPath);
      }
    }
  },

  CatchClause(path) {
    path.scope.registerBinding('let', path);
  },

  Function(path) {
    if (
      path.isFunctionExpression() &&
      path.has('id') &&
      !path.get('id').node[t.NOT_LOCAL_BINDING]
    ) {
      path.scope.registerBinding('local', path.get('id'), path);
    }

    const params: Array<NodePath> = path.get('params');
    for (const param of params) {
      path.scope.registerBinding('param', param);
    }
  },

  ClassExpression(path) {
    if (path.has('id') && !path.get('id').node[t.NOT_LOCAL_BINDING]) {
      path.scope.registerBinding('local', path);
    }
  },
};

let uid = 0;

export default class Scope {
  /**
   * This searches the current "scope" and collects all references/bindings
   * within.
   */

  constructor(path: NodePath) {
    const {node} = path;
    const cached = scopeCache.get(node);
    // Sometimes, a scopable path is placed higher in the AST tree.
    // In these cases, have to create a new Scope.
    if (cached?.path === path) {
      return cached;
    }
    scopeCache.set(node, this);

    this.uid = uid++;

    this.block = node;
    this.path = path;

    this.labels = new Map();
    this.inited = false;
  }

  /**
   * Globals.
   */

  static globals = Object.keys(globals.builtin);

  /**
   * Variables available in current context.
   */

  static contextVariables = ['arguments', 'undefined', 'Infinity', 'NaN'];

  get parent() {
    const parent = this.path.findParent(p => p.isScope());
    return parent?.scope;
  }

  get parentBlock() {
    return this.path.parent;
  }

  get hub() {
    return this.path.hub;
  }

  /**
   * Traverse node with current scope and path.
   */

  traverse(node: Object, opts: Object, state?) {
    traverse(node, opts, this, state, this.path);
  }

  /**
   * Generate a unique identifier and add it to the current scope.
   */

  generateDeclaredUidIdentifier(name?: string) {
    const id = this.generateUidIdentifier(name);
    this.push({id});
    return t.cloneNode(id);
  }

  /**
   * Generate a unique identifier.
   */

  generateUidIdentifier(name?: string) {
    return t.identifier(this.generateUid(name));
  }

  /**
   * Generate a unique `_id1` binding.
   */

  generateUid(name: string = 'temp') {
    name = t
      .toIdentifier(name)
      .replace(/^_+/, '')
      .replace(/[0-9]+$/g, '');

    let uid;
    let i = 1;
    do {
      uid = this._generateUid(name, i);
      i++;
    } while (
      this.hasLabel(uid) ||
      this.hasBinding(uid) ||
      this.hasGlobal(uid) ||
      this.hasReference(uid)
    );

    const program = this.getProgramParent();
    program.references[uid] = true;
    program.uids[uid] = true;

    return uid;
  }

  /**
   * Generate an `_id1`.
   */

  _generateUid(name, i) {
    let id = name;
    if (i > 1) id += i;
    return `_${id}`;
  }

  generateUidBasedOnNode(node: Object, defaultName?: String) {
    const parts = [];
    gatherNodeParts(node, parts);

    let id = parts.join('$');
    id = id.replace(/^_/, '') || defaultName || 'ref';

    return this.generateUid(id.slice(0, 20));
  }

  /**
   * Generate a unique identifier based on a node.
   */

  generateUidIdentifierBasedOnNode(node: Object, defaultName?: String): Object {
    return t.identifier(this.generateUidBasedOnNode(node, defaultName));
  }

  /**
   * Determine whether evaluating the specific input `node` is a consequenceless reference. ie.
   * evaluating it wont result in potentially arbitrary code from being ran. The following are
   * allowed and determined not to cause side effects:
   *
   *  - `this` expressions
   *  - `super` expressions
   *  - Bound identifiers
   */

  isStatic(node: Object): boolean {
    if (t.isThisExpression(node) || t.isSuper(node)) {
      return true;
    }

    if (t.isIdentifier(node)) {
      const binding = this.getBinding(node.name);
      if (binding) {
        return binding.constant;
      } else {
        return this.hasBinding(node.name);
      }
    }

    return false;
  }

  /**
   * Possibly generate a memoised identifier if it is not static and has consequences.
   */

  maybeGenerateMemoised(node: Object, dontPush?: boolean): ?Object {
    if (this.isStatic(node)) {
      return null;
    } else {
      const id = this.generateUidIdentifierBasedOnNode(node);
      if (!dontPush) {
        this.push({id});
        return t.cloneNode(id);
      }
      return id;
    }
  }

  checkBlockScopedCollisions(local, kind: string, name: string, id: Object) {
    // ignore parameters
    if (kind === 'param') return;

    // Ignore existing binding if it's the name of the current function or
    // class expression
    if (local.kind === 'local') return;

    const duplicate =
      // don't allow duplicate bindings to exist alongside
      kind === 'let' ||
      local.kind === 'let' ||
      local.kind === 'const' ||
      local.kind === 'module' ||
      // don't allow a local of param with a kind of let
      (local.kind === 'param' && (kind === 'let' || kind === 'const'));

    if (duplicate) {
      throw this.hub.buildError(
        id,
        `Duplicate declaration "${name}"`,
        TypeError,
      );
    }
  }

  rename(oldName: string, newName: string, block?) {
    const binding = this.getBinding(oldName);
    if (binding) {
      newName = newName || this.generateUidIdentifier(oldName).name;
      return new Renamer(binding, oldName, newName).rename(block);
    }
  }

  _renameFromMap(map, oldName, newName, value) {
    if (map[oldName]) {
      map[newName] = value;
      map[oldName] = null;
    }
  }

  dump() {
    const sep = '-'.repeat(60);
    console.log(sep);
    let scope = this;
    do {
      console.log('#', scope.block.type);
      for (const name of Object.keys(scope.bindings)) {
        const binding = scope.bindings[name];
        console.log(' -', name, {
          constant: binding.constant,
          references: binding.references,
          violations: binding.constantViolations.length,
          kind: binding.kind,
        });
      }
    } while ((scope = scope.parent));
    console.log(sep);
  }

  // TODO: (Babel 8) Split i in two parameters, and use an object of flags
  toArray(node: Object, i?: number | boolean, allowArrayLike?: boolean) {
    if (t.isIdentifier(node)) {
      const binding = this.getBinding(node.name);
      if (binding?.constant && binding.path.isGenericType('Array')) {
        return node;
      }
    }

    if (t.isArrayExpression(node)) {
      return node;
    }

    if (t.isIdentifier(node, {name: 'arguments'})) {
      return t.callExpression(
        t.memberExpression(
          t.memberExpression(
            t.memberExpression(
              t.identifier('Array'),
              t.identifier('prototype'),
            ),
            t.identifier('slice'),
          ),
          t.identifier('call'),
        ),
        [node],
      );
    }

    let helperName;
    const args = [node];
    if (i === true) {
      // Used in array-spread to create an array.
      helperName = 'toConsumableArray';
    } else if (i) {
      args.push(t.numericLiteral(i));

      // Used in array-rest to create an array from a subset of an iterable.
      helperName = 'slicedToArray';
      // TODO if (this.hub.isLoose("es6.forOf")) helperName += "-loose";
    } else {
      // Used in array-rest to create an array
      helperName = 'toArray';
    }

    if (allowArrayLike) {
      args.unshift(this.hub.addHelper(helperName));
      helperName = 'maybeArrayLike';
    }

    return t.callExpression(this.hub.addHelper(helperName), args);
  }

  hasLabel(name: string) {
    return !!this.getLabel(name);
  }

  getLabel(name: string) {
    return this.labels.get(name);
  }

  registerLabel(path: NodePath) {
    this.labels.set(path.node.label.name, path);
  }

  registerDeclaration(path: NodePath) {
    if (path.isLabeledStatement()) {
      this.registerLabel(path);
    } else if (path.isFunctionDeclaration()) {
      this.registerBinding('hoisted', path.get('id'), path);
    } else if (path.isVariableDeclaration()) {
      const declarations = path.get('declarations');
      for (const declar of (declarations: Array)) {
        this.registerBinding(path.node.kind, declar);
      }
    } else if (path.isClassDeclaration()) {
      this.registerBinding('let', path);
    } else if (path.isImportDeclaration()) {
      const specifiers = path.get('specifiers');
      for (const specifier of (specifiers: Array)) {
        this.registerBinding('module', specifier);
      }
    } else if (path.isExportDeclaration()) {
      const declar = path.get('declaration');
      if (
        declar.isClassDeclaration() ||
        declar.isFunctionDeclaration() ||
        declar.isVariableDeclaration()
      ) {
        this.registerDeclaration(declar);
      }
    } else {
      this.registerBinding('unknown', path);
    }
  }

  buildUndefinedNode() {
    return t.unaryExpression('void', t.numericLiteral(0), true);
  }

  registerConstantViolation(path: NodePath) {
    const ids = path.getBindingIdentifiers();
    for (const name of Object.keys(ids)) {
      const binding = this.getBinding(name);
      if (binding) binding.reassign(path);
    }
  }

  registerBinding(kind: string, path: NodePath, bindingPath = path) {
    if (!kind) throw new ReferenceError('no `kind`');

    if (path.isVariableDeclaration()) {
      const declarators: Array<NodePath> = path.get('declarations');
      for (const declar of declarators) {
        this.registerBinding(kind, declar);
      }
      return;
    }

    const parent = this.getProgramParent();
    const ids = path.getOuterBindingIdentifiers(true);

    for (const name of Object.keys(ids)) {
      parent.references[name] = true;

      for (const id of (ids[name]: Array<Object>)) {
        const local = this.getOwnBinding(name);

        if (local) {
          // same identifier so continue safely as we're likely trying to register it
          // multiple times
          if (local.identifier === id) continue;

          this.checkBlockScopedCollisions(local, kind, name, id);
        }

        // A redeclaration of an existing variable is a modification
        if (local) {
          this.registerConstantViolation(bindingPath);
        } else {
          this.bindings[name] = new Binding({
            identifier: id,
            scope: this,
            path: bindingPath,
            kind: kind,
          });
        }
      }
    }
  }

  addGlobal(node: Object) {
    this.globals[node.name] = node;
  }

  hasUid(name): boolean {
    let scope = this;

    do {
      if (scope.uids[name]) return true;
    } while ((scope = scope.parent));

    return false;
  }

  hasGlobal(name: string): boolean {
    let scope = this;

    do {
      if (scope.globals[name]) return true;
    } while ((scope = scope.parent));

    return false;
  }

  hasReference(name: string): boolean {
    return !!this.getProgramParent().references[name];
  }

  isPure(node, constantsOnly?: boolean) {
    if (t.isIdentifier(node)) {
      const binding = this.getBinding(node.name);
      if (!binding) return false;
      if (constantsOnly) return binding.constant;
      return true;
    } else if (t.isClass(node)) {
      if (node.superClass && !this.isPure(node.superClass, constantsOnly)) {
        return false;
      }
      return this.isPure(node.body, constantsOnly);
    } else if (t.isClassBody(node)) {
      for (const method of node.body) {
        if (!this.isPure(method, constantsOnly)) return false;
      }
      return true;
    } else if (t.isBinary(node)) {
      return (
        this.isPure(node.left, constantsOnly) &&
        this.isPure(node.right, constantsOnly)
      );
    } else if (t.isArrayExpression(node)) {
      for (const elem of (node.elements: Array<Object>)) {
        if (!this.isPure(elem, constantsOnly)) return false;
      }
      return true;
    } else if (t.isObjectExpression(node)) {
      for (const prop of (node.properties: Array<Object>)) {
        if (!this.isPure(prop, constantsOnly)) return false;
      }
      return true;
    } else if (t.isMethod(node)) {
      if (node.computed && !this.isPure(node.key, constantsOnly)) return false;
      if (node.kind === 'get' || node.kind === 'set') return false;
      return true;
    } else if (t.isProperty(node)) {
      if (node.computed && !this.isPure(node.key, constantsOnly)) return false;
      return this.isPure(node.value, constantsOnly);
    } else if (t.isUnaryExpression(node)) {
      return this.isPure(node.argument, constantsOnly);
    } else if (t.isTaggedTemplateExpression(node)) {
      return (
        t.matchesPattern(node.tag, 'String.raw') &&
        !this.hasBinding('String', true) &&
        this.isPure(node.quasi, constantsOnly)
      );
    } else if (t.isTemplateLiteral(node)) {
      for (const expression of (node.expressions: Array<Object>)) {
        if (!this.isPure(expression, constantsOnly)) return false;
      }
      return true;
    } else {
      return t.isPureish(node);
    }
  }

  /**
   * Set some arbitrary data on the current scope.
   */

  setData(key, val) {
    return (this.data[key] = val);
  }

  /**
   * Recursively walk up scope tree looking for the data `key`.
   */

  getData(key) {
    let scope = this;
    do {
      const data = scope.data[key];
      if (data != null) return data;
    } while ((scope = scope.parent));
  }

  /**
   * Recursively walk up scope tree looking for the data `key` and if it exists,
   * remove it.
   */

  removeData(key) {
    let scope = this;
    do {
      const data = scope.data[key];
      if (data != null) scope.data[key] = null;
    } while ((scope = scope.parent));
  }

  init() {
    if (!this.inited) {
      this.inited = true;
      this.crawl();
    }
  }

  crawl() {
    const path = this.path;

    this.references = Object.create(null);
    this.bindings = Object.create(null);
    this.globals = Object.create(null);
    this.uids = Object.create(null);
    this.data = Object.create(null);

    // TODO: explore removing this as it should be covered by collectorVisitor
    if (path.isFunction()) {
      if (
        path.isFunctionExpression() &&
        path.has('id') &&
        !path.get('id').node[t.NOT_LOCAL_BINDING]
      ) {
        this.registerBinding('local', path.get('id'), path);
      }

      const params: Array<NodePath> = path.get('params');
      for (const param of params) {
        this.registerBinding('param', param);
      }
    }

    const programParent = this.getProgramParent();
    if (programParent.crawling) return;

    const state = {
      references: [],
      constantViolations: [],
      assignments: [],
    };

    this.crawling = true;
    path.traverse(collectorVisitor, state);
    this.crawling = false;

    // register assignments
    for (const path of state.assignments) {
      // register undeclared bindings as globals
      const ids = path.getBindingIdentifiers();
      for (const name of Object.keys(ids)) {
        if (path.scope.getBinding(name)) continue;
        programParent.addGlobal(ids[name]);
      }

      // register as constant violation
      path.scope.registerConstantViolation(path);
    }

    // register references
    for (const ref of state.references) {
      const binding = ref.scope.getBinding(ref.node.name);
      if (binding) {
        binding.reference(ref);
      } else {
        programParent.addGlobal(ref.node);
      }
    }

    // register constant violations
    for (const path of state.constantViolations) {
      path.scope.registerConstantViolation(path);
    }
  }

  push(opts: {
    id: Object,
    init: ?Object,
    unique: ?boolean,
    _blockHoist: ?number,
    kind: 'var' | 'let',
  }) {
    let path = this.path;

    if (!path.isBlockStatement() && !path.isProgram()) {
      path = this.getBlockParent().path;
    }

    if (path.isSwitchStatement()) {
      path = (this.getFunctionParent() || this.getProgramParent()).path;
    }

    if (path.isLoop() || path.isCatchClause() || path.isFunction()) {
      path.ensureBlock();
      path = path.get('body');
    }

    const unique = opts.unique;
    const kind = opts.kind || 'var';
    const blockHoist = opts._blockHoist == null ? 2 : opts._blockHoist;

    const dataKey = `declaration:${kind}:${blockHoist}`;
    let declarPath = !unique && path.getData(dataKey);

    if (!declarPath) {
      const declar = t.variableDeclaration(kind, []);
      declar._blockHoist = blockHoist;

      [declarPath] = path.unshiftContainer('body', [declar]);
      if (!unique) path.setData(dataKey, declarPath);
    }

    const declarator = t.variableDeclarator(opts.id, opts.init);
    declarPath.node.declarations.push(declarator);
    this.registerBinding(kind, declarPath.get('declarations').pop());
  }

  /**
   * Walk up to the top of the scope tree and get the `Program`.
   */

  getProgramParent() {
    let scope = this;
    do {
      if (scope.path.isProgram()) {
        return scope;
      }
    } while ((scope = scope.parent));
    throw new Error("Couldn't find a Program");
  }

  /**
   * Walk up the scope tree until we hit either a Function or return null.
   */

  getFunctionParent() {
    let scope = this;
    do {
      if (scope.path.isFunctionParent()) {
        return scope;
      }
    } while ((scope = scope.parent));
    return null;
  }

  /**
   * Walk up the scope tree until we hit either a BlockStatement/Loop/Program/Function/Switch or reach the
   * very top and hit Program.
   */

  getBlockParent() {
    let scope = this;
    do {
      if (scope.path.isBlockParent()) {
        return scope;
      }
    } while ((scope = scope.parent));
    throw new Error(
      "We couldn't find a BlockStatement, For, Switch, Function, Loop or Program...",
    );
  }

  /**
   * Walks the scope tree and gathers **all** bindings.
   */

  getAllBindings(): Object {
    const ids = Object.create(null);

    let scope = this;
    do {
      for (const key of Object.keys(scope.bindings)) {
        if (key in ids === false) {
          ids[key] = scope.bindings[key];
        }
      }
      scope = scope.parent;
    } while (scope);

    return ids;
  }

  /**
   * Walks the scope tree and gathers all declarations of `kind`.
   */

  getAllBindingsOfKind(): Object {
    const ids = Object.create(null);

    for (const kind of (arguments: Array)) {
      let scope = this;
      do {
        for (const name of Object.keys(scope.bindings)) {
          const binding = scope.bindings[name];
          if (binding.kind === kind) ids[name] = binding;
        }
        scope = scope.parent;
      } while (scope);
    }

    return ids;
  }

  bindingIdentifierEquals(name: string, node: Object): boolean {
    return this.getBindingIdentifier(name) === node;
  }

  getBinding(name: string) {
    let scope = this;
    let previousPath;

    do {
      const binding = scope.getOwnBinding(name);
      if (binding) {
        // Check if a pattern is a part of parameter expressions.
        // Note: for performance reason we skip checking previousPath.parentPath.isFunction()
        // because `scope.path` is validated as scope in packages/babel-types/src/validators/isScope.js
        // That is, if a scope path is pattern, its parent must be Function/CatchClause

        // Spec 9.2.10.28: The closure created by this expression should not have visibility of
        // declarations in the function body. If the binding is not a `param`-kind,
        // then it must be defined inside the function body, thus it should be skipped
        if (previousPath?.isPattern() && binding.kind !== 'param') {
          // do nothing
        } else {
          return binding;
        }
      }
      previousPath = scope.path;
    } while ((scope = scope.parent));
  }

  getOwnBinding(name: string) {
    return this.bindings[name];
  }

  getBindingIdentifier(name: string) {
    return this.getBinding(name)?.identifier;
  }

  getOwnBindingIdentifier(name: string) {
    const binding = this.bindings[name];
    return binding?.identifier;
  }

  hasOwnBinding(name: string) {
    return !!this.getOwnBinding(name);
  }

  hasBinding(name: string, noGlobals?) {
    if (!name) return false;
    if (this.hasOwnBinding(name)) return true;
    if (this.parentHasBinding(name, noGlobals)) return true;
    if (this.hasUid(name)) return true;
    if (!noGlobals && Scope.globals.includes(name)) return true;
    if (!noGlobals && Scope.contextVariables.includes(name)) return true;
    return false;
  }

  parentHasBinding(name: string, noGlobals?) {
    return this.parent?.hasBinding(name, noGlobals);
  }

  /**
   * Move a binding of `name` to another `scope`.
   */

  moveBindingTo(name, scope) {
    const info = this.getBinding(name);
    if (info) {
      info.scope.removeOwnBinding(name);
      info.scope = scope;
      scope.bindings[name] = info;
    }
  }

  removeOwnBinding(name: string) {
    delete this.bindings[name];
  }

  removeBinding(name: string) {
    // clear literal binding
    this.getBinding(name)?.scope.removeOwnBinding(name);

    // clear uids with this name - https://github.com/babel/babel/issues/2101
    let scope = this;
    do {
      if (scope.uids[name]) {
        scope.uids[name] = false;
      }
    } while ((scope = scope.parent));
  }
}
