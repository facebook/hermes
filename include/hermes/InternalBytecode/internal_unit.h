/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_INTERNALBYTECODE_INTERNALUNIT_H
#define HERMES_INTERNALBYTECODE_INTERNALUNIT_H

/// A pre-compiled SHUnit to be included with the VM upon construction. This
/// module must be run before any user code can be run.
extern "C" SHUnit internal_unit;

#endif // HERMES_INTERNALBYTECODE_INTERNALUNIT_H
