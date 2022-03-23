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

import type {BlockScope} from './BlockScope';
import type {CatchScope} from './CatchScope';
import type {ClassFieldInitializerScope} from './ClassFieldInitializerScope';
import type {ClassScope} from './ClassScope';
import type {ClassStaticBlockScope} from './ClassStaticBlockScope';
import type {DeclareModuleScope} from './DeclareModuleScope';
import type {ForScope} from './ForScope';
import type {FunctionExpressionNameScope} from './FunctionExpressionNameScope';
import type {FunctionScope} from './FunctionScope';
import type {GlobalScope} from './GlobalScope';
import type {ModuleScope} from './ModuleScope';
import type {SwitchScope} from './SwitchScope';
import type {TypeScope} from './TypeScope';
import type {WithScope} from './WithScope';

type Scope =
  | BlockScope
  | CatchScope
  | ClassFieldInitializerScope
  | ClassScope
  | ClassStaticBlockScope
  | DeclareModuleScope
  | ForScope
  | FunctionExpressionNameScope
  | FunctionScope
  | GlobalScope
  | ModuleScope
  | SwitchScope
  | TypeScope
  | WithScope;

export type {Scope};
