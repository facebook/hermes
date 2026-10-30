/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JIT/Config.h"
#if HERMESVM_JIT
#include "hermes/VM/JIT/arm64/JIT.h"

#include "JitImpl.h"

#define DEBUG_TYPE "jit"

namespace hermes {
namespace vm {
namespace arm64 {

JITContext::JITContext(bool enable) : enabled_(enable) {
  if (!enable)
    return;
  impl_ = std::make_unique<Impl>();
}

JITContext::~JITContext() = default;

void JITContext::setHCIdLimit(uint32_t hcIdLimit) {
  if (impl_)
    impl_->hcIdLimit = std::min<uint32_t>(hcIdLimit, Impl::kHCIdOverflow);
}

void JITContext::dumpCounters(llvh::raw_ostream &os) {
  static constexpr const char *kCounterNames[] = {
#define COUNTER_NAME(name) #name,
      JIT_COUNTERS(COUNTER_NAME)
#undef COUNTER_NAME
  };
  for (unsigned i = 0; i < (unsigned)JitCounter::_Last; ++i)
    os << kCounterNames[i] << ": " << counters_[i] << "\n";
}

void JITContext::markRoots(
    RootAcceptorWithNames &acceptor,
    bool markLongLived) {
  if (!impl_)
    return;
  acceptor.accept(impl_->usedHCs);
}

} // namespace arm64
} // namespace vm
} // namespace hermes
#endif // HERMESVM_JIT
