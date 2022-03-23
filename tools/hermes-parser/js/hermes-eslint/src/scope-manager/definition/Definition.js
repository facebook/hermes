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

import type {CatchClauseDefinition} from './CatchClauseDefinition';
import type {ClassNameDefinition} from './ClassNameDefinition';
import type {EnumDefinition} from './EnumDefinition';
import type {FunctionNameDefinition} from './FunctionNameDefinition';
import type {ImplicitGlobalVariableDefinition} from './ImplicitGlobalVariableDefinition';
import type {ImportBindingDefinition} from './ImportBindingDefinition';
import type {ParameterDefinition} from './ParameterDefinition';
import type {TypeDefinition} from './TypeDefinition';
import type {TypeParameterDefinition} from './TypeParameterDefinition';
import type {VariableDefinition} from './VariableDefinition';

type Definition =
  | CatchClauseDefinition
  | ClassNameDefinition
  | FunctionNameDefinition
  | ImplicitGlobalVariableDefinition
  | ImportBindingDefinition
  | ParameterDefinition
  | EnumDefinition
  | TypeDefinition
  | TypeParameterDefinition
  | VariableDefinition;

export type {Definition};
