/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GCDECL_H
#define HERMES_VM_GCDECL_H

#include "hermes/VM/GCConcurrency.h"

namespace hermes {
namespace vm {

#if defined(HERMESVM_GC_MALLOC)
class MallocGC;
using GC = MallocGC;
#elif defined(HERMESVM_GC_HADES)
class HadesGC;
using GC = HadesGC;
#elif defined(HERMESVM_GC_RUNTIME)
class GCBase;
class HadesGC;
class MallocGC;
using GC = GCBase;
#else
#error "Unsupported HermesVM GCKIND" #HERMESVM_GCKIND
#endif

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_GCDECL_H
