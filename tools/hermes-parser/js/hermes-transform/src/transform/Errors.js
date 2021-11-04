/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ESNode} from 'hermes-estree';

export class NodeIsDeletedError extends Error {}
export class NodeIsMutatedError extends Error {}
