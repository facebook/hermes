/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT_X86_64
#include "JitEmitter-internal.h"
#include "JitEmitter.h"

namespace hermes::vm::x86_64 {

void Emitter::ret(FR frValue) {
  // kGPReturnStash (rbx) is the return value stash (the x21 analogue):
  // leave() moves it to rax, the SysV return register, after restoring the
  // frame. Clobbering rbx here without invalidating any FR's HWReg state is
  // sound only because every path through this mov terminates at leave():
  // there is no fall-through or branch back into the FR allocator that
  // could observe rbx's old, now-stale contents.
  movHWFromFR(HWReg::gpX(kGPReturnStash), frValue);
  a.jmp(returnLabel_);
}

} // namespace hermes::vm::x86_64

#endif // HERMESVM_JIT_X86_64
