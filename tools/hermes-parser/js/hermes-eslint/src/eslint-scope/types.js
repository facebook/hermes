/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

import type ScopeManagerClass from './scope-manager';
import type VariableClass from './variable';

export type {Definition} from './definition';
export type {Reference} from './reference';
export type {Scope} from './scope';
export type ScopeManager = ScopeManagerClass;
export type Variable = VariableClass;
