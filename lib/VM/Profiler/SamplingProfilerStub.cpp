/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Profiler/SamplingProfiler.h"

#ifdef HERMESVM_SAMPLING_PROFILER_STUB

namespace hermes {
namespace vm {

/*static*/ const std::shared_ptr<SamplingProfiler>
    &SamplingProfiler::getInstance() {
  // Do not use make_shared here because that requires
  // making constructor public.
  static std::shared_ptr<SamplingProfiler> instance(new SamplingProfiler());
  return instance;
}

} // namespace vm
} // namespace hermes

#endif // HERMESVM_SAMPLING_PROFILER_STUB
