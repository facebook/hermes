/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/instrumentation/ProcessStats.h"

#include "gtest/gtest.h"

#include <sys/mman.h>
#include <unistd.h>
#include <chrono>
#include <functional>

using namespace hermes::vm;

namespace {

/// Test that the statistics in \p actual differ from \p initial by the supplied
/// delta values:
///
///     actual.RSSkB == initial.RSSkB + dRSSkB
///     actual.VAkB == initial.VAkB + dVAkB
///
/// This function is called via a macro which populates the following
/// parameters, which are used to augment test failure output:
///
/// \p file The filename of the callee.
/// \p line The line number in the file of the callee.
/// \p initExpr A string representation of the expression that was evaluated to
///     compute \p initial
/// \p actualExpr A string representation of the expression that was evaluated
///     to compute \p actual.
void infoAssertionImpl(
    const char *file,
    unsigned line,
    const char *initExpr,
    const ProcessStats::Info &initial,
    const char *actualExpr,
    const ProcessStats::Info &actual,
    int64_t dRSSkB,
    int64_t dVAkB);

using InfoAssertion = std::function<decltype(infoAssertionImpl)>;

/// Certain platforms (e.g. linux) do not keep global RSS counts up-to-date
/// strictly in sync with the events that might change them.  This function
/// encourages the counters to become synchronised with the state of the world.
void flushRSSEvents();

/// Dirty every page intersecting with the range [from to), by writing to it.
void touchPages(char *from, char *to);

/// Run through the motions of the ProcessStats test, using \p EXPECT_INFO_DELTA
/// as the assertion function to check the delta between ProcessStats::Info
/// structs.
void ProcessStatsTest(InfoAssertion assertionImpl);

TEST(ProcessStatsTest, Test) {
  // Run the test without checking anything.  This ensures that all the code for
  // the test is paged in now, and not later when we are watching the resident
  // set size.
  ProcessStatsTest([](const char *file,
                      unsigned line,
                      const char *initExpr,
                      const ProcessStats::Info &initial,
                      const char *actualExpr,
                      const ProcessStats::Info &actual,
                      int64_t dRSSkB,
                      int64_t dVAkB) {});

  // Run again, this time checking the change in values.
  ProcessStatsTest(infoAssertionImpl);
}

void ProcessStatsTest(InfoAssertion assertionImpl) {
  const size_t PS = getpagesize();
  const size_t PSkB = PS / 1024;

  flushRSSEvents();
  ProcessStats stats;

  const std::chrono::milliseconds kTimeStep(10);
  auto now = stats.initTime();

  auto takeSample = [&stats, &now, kTimeStep]() {
    flushRSSEvents();
    stats.sample(now);
    now += kTimeStep;
    return stats.getIntegratedInfo();
  };

  const auto initial = takeSample();

  char *buf = reinterpret_cast<char *>(mmap(
      nullptr, 10 * PS, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0));
  ASSERT_NE(MAP_FAILED, buf);

  const auto afterMmap = takeSample();

  touchPages(buf, buf + 5 * PS);

  const auto afterTouch = takeSample();
  const auto afterNoop = takeSample();

  touchPages(buf, buf + 10 * PS);

  const auto afterTouchAll = takeSample();

  auto unmap = munmap(buf, 10 * PS);
  ASSERT_NE(-1, unmap);

  const auto afterUnmap = takeSample();
  const auto afterNoop2 = takeSample();

#define EXPECT_INFO_DELTA(BEFORE, AFTER, DRSSKB, DVAKB) \
  assertionImpl(                                        \
      __FILE__,                                         \
      __LINE__,                                         \
      #BEFORE,                                          \
      (BEFORE),                                         \
      #AFTER,                                           \
      (AFTER),                                          \
      (DRSSKB),                                         \
      (DVAKB))

  // Many of the deltas we check against below are divided by 2.  This comes
  // from the fact they are the delta of an area we approximate using the
  // trapezoidal rule. For example, if we sample a function f at 0 and 1 such
  // that f(0) = 0, f(1) = N, we expect the approximation of the area under f
  // between 0 and 1 to be N/2.

  EXPECT_INFO_DELTA(initial, afterMmap, 0, 10 * PSkB * kTimeStep.count() / 2);

  EXPECT_INFO_DELTA(
      afterMmap,
      afterTouch,
      5 * PSkB * kTimeStep.count() / 2,
      10 * PSkB * kTimeStep.count());

  EXPECT_INFO_DELTA(
      afterTouch,
      afterNoop,
      5 * PSkB * kTimeStep.count(),
      10 * PSkB * kTimeStep.count());

  EXPECT_INFO_DELTA(
      afterNoop,
      afterTouchAll,
      5 * PSkB * kTimeStep.count() * 3 / 2,
      10 * PSkB * kTimeStep.count());

  EXPECT_INFO_DELTA(
      afterTouchAll,
      afterUnmap,
      10 * PSkB * kTimeStep.count() / 2,
      10 * PSkB * kTimeStep.count() / 2);

  EXPECT_INFO_DELTA(afterUnmap, afterNoop2, 0, 0);

#undef EXPECT_INFO_DELTA
}

void infoAssertionImpl(
    const char *file,
    unsigned line,
    const char *initExpr,
    const ProcessStats::Info &initial,
    const char *actualExpr,
    const ProcessStats::Info &actual,
    int64_t dRSSkB,
    int64_t dVAkB) {
  EXPECT_EQ(initial.RSSkB + dRSSkB, actual.RSSkB)
      << "At " << file << ":" << line << "\n"
      << "  " << initExpr << " -> " << actualExpr << "\n"
      << "  RSS: " << initial.RSSkB << " + " << dRSSkB
      << " != " << actual.RSSkB;
  EXPECT_EQ(initial.VAkB + dVAkB, actual.VAkB)
      << "At " << file << ":" << line << "\n"
      << "  " << initExpr << " -> " << actualExpr << "\n"
      << "  VA:  " << initial.VAkB << " + " << dVAkB << " != " << actual.VAkB;
}

void flushRSSEvents() {
  const size_t PS = getpagesize();
  auto buf =
      mmap(nullptr, PS, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
  assert(buf != MAP_FAILED);

  auto p = reinterpret_cast<volatile char *>(buf);

  // Page it in
  *p = 1;

  // Page it out
  madvise(buf, PS, MADV_DONTNEED);

  auto unmap = munmap(buf, PS);
  assert(unmap != -1);
  (void)unmap;
}

void touchPages(char *from, char *to) {
  const size_t PS = getpagesize();
  for (volatile char *p = from; p < to; p += PS)
    *p = 1;
}

} // namespace
