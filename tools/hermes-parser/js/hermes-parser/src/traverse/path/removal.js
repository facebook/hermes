/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// This file contains methods responsible for removing a node.

import {hooks} from './lib/removal-hooks';
import {path as pathCache} from '../cache';
import {REMOVED, SHOULD_SKIP} from './index';

export function remove() {
  this._assertUnremoved();

  this.resync();
  if (!this.opts?.noScope) {
    this._removeFromScope();
  }

  if (this._callRemovalHooks()) {
    this._markRemoved();
    return;
  }

  this.shareCommentsWithSiblings();
  this._remove();
  this._markRemoved();
}

export function _removeFromScope() {
  const bindings = this.getBindingIdentifiers();
  Object.keys(bindings).forEach(name => this.scope.removeBinding(name));
}

export function _callRemovalHooks() {
  for (const fn of (hooks: Array<Function>)) {
    if (fn(this, this.parentPath)) return true;
  }
}

export function _remove() {
  if (Array.isArray(this.container)) {
    this.container.splice(this.key, 1);
    this.updateSiblingKeys(this.key, -1);
  } else {
    this._replaceWith(null);
  }
}

export function _markRemoved() {
  // this.shouldSkip = true; this.removed = true;
  this._traverseFlags |= SHOULD_SKIP | REMOVED;
  if (this.parent) pathCache.get(this.parent).delete(this.node);
  this.node = null;
}

export function _assertUnremoved() {
  if (this.removed) {
    throw this.buildCodeFrameError(
      'NodePath has been removed so is read-only.',
    );
  }
}
