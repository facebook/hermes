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

import type {Identifier} from 'hermes-estree';
import type {Definition} from './definition';
import type {Reference} from './reference';
import type {Scope} from './scope';

/**
 * A Variable represents a locally scoped identifier. These include arguments to
 * functions.
 * @class Variable
 */
class Variable {
  /**
   * The variable name, as given in the source code.
   */
  name: string;

  /**
   * List of defining occurrences of this variable (like in 'var ...'
   * statements or as parameter), as AST nodes.
   */
  identifiers: Array<Identifier> = [];

  /**
   * List of references of this variable (excluding parameter entries)
   * in its defining scope and all nested scopes. For defining
   * occurrences only see defs.
   */
  references: Array<Reference> = [];

  /**
   * List of defining occurrences of this variable (like in 'var ...'
   * statements or as parameter), as custom objects.
   */
  defs: Array<Definition> = [];

  /**
   * Reference to the enclosing Scope.
   */
  scope: Scope;

  constructor(name: string, scope: Scope) {
    this.name = name;
    this.scope = scope;
  }
}

module.exports = Variable;
