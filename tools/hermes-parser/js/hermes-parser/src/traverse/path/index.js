/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import type {HubInterface} from '../hub';
import type TraversalContext from '../context';
import * as virtualTypes from './lib/virtual-types';
import buildDebug from 'debug';
import traverse from '../index';
import Scope from '../scope';
import * as t from '../../types';
import {path as pathCache} from '../cache';
import generator from '@babel/generator';

// NodePath is split across many files.
import * as NodePath_ancestry from './ancestry';
import * as NodePath_inference from './inference';
import * as NodePath_replacement from './replacement';
import * as NodePath_evaluation from './evaluation';
import * as NodePath_conversion from './conversion';
import * as NodePath_introspection from './introspection';
import * as NodePath_context from './context';
import * as NodePath_removal from './removal';
import * as NodePath_modification from './modification';
import * as NodePath_family from './family';
import * as NodePath_comments from './comments';

const debug = buildDebug('babel');

export const REMOVED = 1 << 0;
export const SHOULD_STOP = 1 << 1;
export const SHOULD_SKIP = 1 << 2;

export default class NodePath {
  constructor(hub: HubInterface, parent: Object) {
    this.parent = parent;
    this.hub = hub;
    this.data = null;

    this.context = null;
    this.scope = null;
  }

  declare parent: Object;
  declare hub: HubInterface;
  declare data: Object;
  declare context: TraversalContext;
  declare scope: Scope;

  contexts: Array<TraversalContext> = [];
  state: any = null;
  opts: ?Object = null;
  // this.shouldSkip = false; this.shouldStop = false; this.removed = false;
  _traverseFlags: number = 0;
  skipKeys: ?Object = null;
  parentPath: ?NodePath = null;
  container: ?Object | Array<Object> = null;
  listKey: ?string = null;
  key: ?string = null;
  node: ?Object = null;
  type: ?string = null;

  static get({hub, parentPath, parent, container, listKey, key}): NodePath {
    if (!hub && parentPath) {
      hub = parentPath.hub;
    }

    if (!parent) {
      throw new Error('To get a node path the parent needs to exist');
    }

    const targetNode = container[key];

    let paths = pathCache.get(parent);
    if (!paths) {
      paths = new Map();
      pathCache.set(parent, paths);
    }

    let path = paths.get(targetNode);
    if (!path) {
      path = new NodePath(hub, parent);
      if (targetNode) paths.set(targetNode, path);
    }

    path.setup(parentPath, container, listKey, key);

    return path;
  }

  getScope(scope: Scope) {
    return this.isScope() ? new Scope(this) : scope;
  }

  setData(key: string, val: any): any {
    if (this.data == null) {
      this.data = Object.create(null);
    }
    return (this.data[key] = val);
  }

  getData(key: string, def?: any): any {
    if (this.data == null) {
      this.data = Object.create(null);
    }
    let val = this.data[key];
    if (val === undefined && def !== undefined) val = this.data[key] = def;
    return val;
  }

  buildCodeFrameError(msg: string, Error: typeof Error = SyntaxError): Error {
    return this.hub.buildError(this.node, msg, Error);
  }

  traverse(visitor: Object, state?: any) {
    traverse(this.node, visitor, this.scope, state, this);
  }

  set(key: string, node: Object) {
    t.validate(this.node, key, node);
    this.node[key] = node;
  }

  getPathLocation(): string {
    const parts = [];
    let path = this;
    do {
      let key = path.key;
      if (path.inList) key = `${path.listKey}[${key}]`;
      parts.unshift(key);
    } while ((path = path.parentPath));
    return parts.join('.');
  }

  debug(message) {
    if (!debug.enabled) return;
    debug(`${this.getPathLocation()} ${this.type}: ${message}`);
  }

  toString() {
    return generator(this.node).code;
  }

  get inList() {
    return !!this.listKey;
  }

  set inList(inList) {
    if (!inList) {
      this.listKey = null;
    }
    // ignore inList = true as it should depend on `listKey`
  }

  get parentKey() {
    return this.listKey || this.key;
  }

  get shouldSkip() {
    return !!(this._traverseFlags & SHOULD_SKIP);
  }

  set shouldSkip(v) {
    if (v) {
      this._traverseFlags |= SHOULD_SKIP;
    } else {
      this._traverseFlags &= ~SHOULD_SKIP;
    }
  }

  get shouldStop() {
    return !!(this._traverseFlags & SHOULD_STOP);
  }

  set shouldStop(v) {
    if (v) {
      this._traverseFlags |= SHOULD_STOP;
    } else {
      this._traverseFlags &= ~SHOULD_STOP;
    }
  }

  get removed() {
    return !!(this._traverseFlags & REMOVED);
  }
  set removed(v) {
    if (v) {
      this._traverseFlags |= REMOVED;
    } else {
      this._traverseFlags &= ~REMOVED;
    }
  }
}

Object.assign(
  NodePath.prototype,
  NodePath_ancestry,
  NodePath_inference,
  NodePath_replacement,
  NodePath_evaluation,
  NodePath_conversion,
  NodePath_introspection,
  NodePath_context,
  NodePath_removal,
  NodePath_modification,
  NodePath_family,
  NodePath_comments,
);

for (const type of (t.TYPES: Array<string>)) {
  const typeKey = `is${type}`;
  const fn = t[typeKey];
  NodePath.prototype[typeKey] = function (opts) {
    return fn(this.node, opts);
  };

  NodePath.prototype[`assert${type}`] = function (opts) {
    if (!fn(this.node, opts)) {
      throw new TypeError(`Expected node path of type ${type}`);
    }
  };
}

for (const type of Object.keys(virtualTypes)) {
  if (type[0] === '_') continue;
  if (t.TYPES.indexOf(type) < 0) t.TYPES.push(type);

  const virtualType = virtualTypes[type];

  NodePath.prototype[`is${type}`] = function (opts) {
    return virtualType.checkPath(this, opts);
  };
}
