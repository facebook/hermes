/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "cachenewobject"

#include "hermes/Optimizer/Scalar/CacheNewObject.h"

#include "hermes/IR/Instrs.h"

using namespace hermes;

bool CacheNewObject::runOnFunction(Function *func) {
  return false;
}

Pass *hermes::createCacheNewObject() {
  return new CacheNewObject();
}

#undef DEBUG_TYPE
