/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Public/Buffer.h"
#include "hermes/Public/CrashManager.h"
#include "hermes/Public/GCTripwireContext.h"
#include "hermes/Public/JSOutOfMemoryError.h"

/// This file provides "key functions" for types in the API that have virtual
/// methods. This ensures that the type information for these types is only
/// generated and exported from one place, to avoid bugs arising from duplicate
/// type information across shared library boundaries.

namespace hermes {
Buffer::~Buffer() {}
namespace vm {
GCTripwireContext::~GCTripwireContext() {}
CrashManager::~CrashManager() {}
NopCrashManager::~NopCrashManager() {}
JSOutOfMemoryError::~JSOutOfMemoryError() {}
} // namespace vm
} // namespace hermes
