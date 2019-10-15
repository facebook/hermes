/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_INSTRUMENTATION_PERFMARKER_H
#define HERMES_INSTRUMENTATION_PERFMARKER_H

#ifdef HERMES_SAMPLED_INSTRUMENTATION
#include <perf/instrumentation/metrics/KernelCounters.h>
#include <perf/instrumentation/sampled/marker.h>
#endif

namespace hermes {
namespace instrumentation {

#ifdef HERMES_SAMPLED_INSTRUMENTATION
using LowFreqPerfMarker =
    facebook::perf::instrumentation::sampled::LowFreqMarker<
        facebook::perf::instrumentation::metrics::KernelCounters>;
using PerfMarker = facebook::perf::instrumentation::sampled::Marker<
    facebook::perf::instrumentation::metrics::KernelCounters>;
using HighFreqPerfMarker =
    facebook::perf::instrumentation::sampled::HighFreqMarker<
        facebook::perf::instrumentation::metrics::KernelCounters>;

#else
struct MarkerImpl {
  MarkerImpl(const char *) {}
  MarkerImpl(const MarkerImpl &) = delete;
  MarkerImpl(MarkerImpl &&) = delete;

  MarkerImpl &operator=(const MarkerImpl &) = delete;
  MarkerImpl &operator=(MarkerImpl &&) = delete;
};

using LowFreqPerfMarker = MarkerImpl;
using PerfMarker = MarkerImpl;
using HighFreqPerfMarker = MarkerImpl;
#endif

} // namespace instrumentation
} // namespace hermes

#endif
