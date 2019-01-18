/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef HERMES_FACEBOOK_BUILD
#define DEBUG_TYPE "gc"
#include "hermes/VM/GC.h"

#if defined(__ANDROID__)
#include "glue/BreakpadCustomData.h"
#endif

namespace hermes {
namespace vm {

/*static*/
void GCBase::oomDetailFB(char *detail) {
#if defined(__ANDROID__)
  setBreakpadCustomData("HermesGCOOMDetailBasic", "%s", detail);
#endif
}

} // namespace vm
} // namespace hermes
#endif
