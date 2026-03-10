/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_GC_H
#define HERMES_VM_GC_H

#include "hermes/VM/sh_config.h"

#if HERMESVM_GCKIND == _HERMESVM_GCVALUE_MALLOC
#include "hermes/VM/MallocGC.h"
#elif HERMESVM_GCKIND == _HERMESVM_GCVALUE_HADES
#include "hermes/VM/HadesGC.h"
#else
#error "Unsupported HermesVM GCKIND" #HERMESVM_GCKIND
#endif

#endif // HERMES_VM_GC_H
