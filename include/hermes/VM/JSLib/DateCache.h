/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_DATECACHE_H
#define HERMES_DATECACHE_H

#include "hermes/VM/JSLib/DateUtil.h"

namespace hermes {
namespace vm {

/// Type of time represented by an epoch.
enum class TimeType : int8_t {
  Local,
  Utc,
};

/// Cache local time offset (including day light saving offset).
///
/// Standard offset is computed via localTZA() and cached. DST is cached in an
/// array of time intervals, time in the same interval has a fixed DST offset.
/// For every new time point that is not included in any existing time interval,
/// compute its DST offset, try extending an existing interval, or creating a
/// new one and storing to the cache array (replace the least recently used
/// cache if it's full).
///
/// The algorithm of the DST caching is described below:
/// 1. Initialize every interval in the DST cache array to be [t_max, -t_max],
/// which is called "empty" interval, here t_max is the maximum epoch time
/// supported by some OS time APIs. Initialize R_before and R_after to point to
/// the first two intervals in the array. And constant Delta is the maximum time
/// range that can have at most one DST transition. Each entry also has an epoch
/// value that is used to find out the least recently used entry, but we omit
/// the operations on them in this description.
/// 2. Give a UTC time point t, check if it's included in R_before, if yes,
/// return the DST offset of R_before. Otherwise, go to step 3.
/// 3. Search the cache array and try to find two intervals:
///    [s_before, e_before] where s_before <= t and as close as possible.
///    [s_after, e_after] where t < e_after and as close as possible.
///    If one interval is not found, let it point to an empty interval in the
///    cache array (if no empty interval in it, reset the least recently used
///    one to empty and use it). Then assign the two intervals to R_before and
///    R_after respectively.
/// 4. Check if the new R_before is empty, if yes, compute the DST of t, assign
///    it to R_before, and set the interval of R_before to [t, t]. Otherwise, go
///    to step 5.
/// 5. Check if t is included in the new non-empty R_before, if yes, return its
///    DST. Otherwise, go to step 6.
/// 6. If t > R_new, where R_new = R_before.end + Delta, compute the DST offset
///    for t and call extendOrRecomputeAfterCache(t, offset) (described later)
///    to update R_after, then swap R_after and R_before, return offset.
///    Otherwise, go to step 7.
/// 7. If t <= R_new < R_after.start, compute the DST offset for t and call
///    extendOrRecomputeAfterCache(t, offset) to update R_after.
/// 8. If R_before.offset == R_after.offset, merge R_after to R_before and
///    reset R_after, then return the offset. Otherwise, go to step 9.
/// 9. At this step, there must been one DST transition in interval
///    (R_before.end, R_after.start], compute DST of t and do binary search to
///    find a time point that has the same offset as t, and extend R_before or
///    R_after to it. In the end, return the offset.
/// 10. Given a new UTC time point, repeat step 2 - 9.
///
/// Algorithm for extendOrRecomputeAfterCache(t, offset):
/// 1. If offset == R_after.offset and R_after.start-Delta <= t <= R_after.end,
///    let R_after.start = t and return.
/// 2. If R_after is not empty, scan every entry in the cache array (except
///    R_before), find out the least recently used one, reset it to empty.
/// 3. Assign offset to R_after and update its interval to be [t, t].
///
/// Note that on Linux, the standard offset and DST offset is unchanged even if
/// TZ is updated, since the underlying time API in C library caches the time
/// zone. On Windows, currently we don't detect TZ changes as well. But this
/// could change if we migrate the usage of C API to Win32 API. On MacOS, the
/// time API does not cache, so we will check if the standard offset has changed
/// in computeDaylightSaving(), and reset both the standard offset cache and DST
/// cache in the next call to getLocalTimeOffset() or daylightSavingOffsetInMs()
/// (the current call will still use the old TZ). This is to ensure that they
/// are consistent w.r.t. the current TZ.
class LocalTimeOffsetCache {
 public:
  /// All runtime functionality should use the instance provided in
  /// JSLibStorage, this is public only because we need to construct a instance
  /// in the unit tests (alternatively, we have to create a singleton function
  /// to be used in the tests).
  LocalTimeOffsetCache() {
    reset();
  }

  /// Reset the standard local time offset and the DST cache.
  void reset() {
    ::tzset();
    ltza_ = localTZA();
    caches_.fill(DSTCacheEntry{});
    before_ = caches_.data();
    after_ = caches_.data() + 1;
    epoch_ = 0;
    needsToReset_ = false;
  }

  /// Compute local timezone offset (DST included).
  /// \param timeMs time in milliseconds.
  /// \param timeType whether \p timeMs is UTC or local time.
  double getLocalTimeOffset(double timeMs, TimeType timeType);

  /// \param time_ms UTC epoch in milliseconds.
  /// \return Daylight saving offset at time \p time_ms.
  int daylightSavingOffsetInMs(int64_t timeMs);

 private:
  LocalTimeOffsetCache(const LocalTimeOffsetCache &) = delete;
  LocalTimeOffsetCache operator=(const LocalTimeOffsetCache &) = delete;

  /// Number of cached time intervals for DST.
  static constexpr unsigned kCacheSize = 32;
  /// Default length of each time interval. The implementation relies on the
  /// fact that no time zones have more than one daylight savings offset change
  /// per 19 days. In Egypt in 2010 they decided to suspend DST during Ramadan.
  /// This led to a short interval where DST is in effect from September 10 to
  /// September 30.
  static constexpr int64_t kDSTDeltaMs = 19 * SECONDS_PER_DAY * MS_PER_SECOND;
  // The largest time that can be passed to OS date-time library functions.
  static constexpr int64_t kMaxEpochTimeInMs =
      std::numeric_limits<int32_t>::max() * MS_PER_SECOND;

  struct DSTCacheEntry {
    /// Start and end time of this DST cache interval, in UTC time.
    int64_t startMs{kMaxEpochTimeInMs};
    int64_t endMs{-kMaxEpochTimeInMs};
    /// The DST offset in [startMs, endMs].
    int dstOffsetMs{0};
    /// Used for LRU.
    int epoch{0};

    /// \return whether this is a valid interval.
    bool isEmpty() const {
      return startMs > endMs;
    }
  };

  /// Compute the DST offset at UTC time \p timeMs.
  /// Note that this may update needsToReset_ if it detects a different
  /// standard offset than the cached one.
  int computeDaylightSaving(int64_t timeMs);

  /// Increase the epoch counter and return it.
  int bumpEpoch() {
    ++epoch_;
    return epoch_;
  }

  /// Find the least recently used DST cache and reuse it.
  /// \param skip do not scan this cache. Technically, it can be nullptr, which
  /// means we don't skip any entry in the cache array. But currently we always
  /// passed in an meaningful entry pointer (mostly before_ or after_).
  /// \return an empty DST cache. This should never return nullptr.
  DSTCacheEntry *leastRecentlyUsedExcept(const DSTCacheEntry *const skip);

  /// Scan the DST caches to find a cached interval starts at or before \p
  /// timeMs (as late as possible) and an interval ends after \p timeMs (as
  /// early as possible). Set the pointer to before_ and after_ (it must be true
  /// that before_ != after_). If none is found, reuse an empty cache.
  void findBeforeAndAfterEntries(int64_t timeMs);

  /// If after_->dstOffsetMs == \p dstOffsetMs and \p timeMs is included in
  /// [after_->startMs - kDSTDeltaMs, after_->endMs], extend after_ to
  /// [timeMs, after_->endMs].
  /// Otherwise, let after_ point to the least recently used cache entry and
  /// update its interval to be [timeMs, timeMs], and its DST offset to be
  /// \p dstOffsetMs.
  void extendOrRecomputeAfterCache(int64_t timeMs, int dstOffsetMs);

  /// Integer counter used to find least recently used cache.
  int epoch_;
  /// Point to one entry in caches_ array, and this invariant always
  /// holds: before_->startMs <= t <= before_->endMs, where t is the last seen
  /// time point.
  DSTCacheEntry *before_;
  /// Point to one entry in the caches_ array, and must be different from
  /// before_. Unlike before, this interval does not always end after the last
  /// seen time point. Instead, each time after findBeforeAndAfterEntries() is
  /// called, after_ is updated to an interval that ends after the time point,
  /// and each time after extendOrRecomputeAfterCache() is called, it's updated
  /// to an interval that includes the time point. At any other steps, there is
  /// no invariant between after_ and the last seen time point.
  DSTCacheEntry *after_;
  /// A list of cached intervals computed for previously seen time points.
  /// In the beginning, each interval is initialized to empty. When every
  /// interval is non-empty and we need a new empty one, reset the least
  /// recently used one (by comparing the epoch value) to empty.
  std::array<DSTCacheEntry, kCacheSize> caches_;
  /// The standard local timezone offset (without DST offset).
  int64_t ltza_;
  /// Whether needs to reset the cache and ltza_.
  /// We don't do reset in the middle of DaylightSavingOffsetInMs() because it
  /// may cause that function never return if another thread is keeping updating
  /// TZ. But that means we may return incorrect result before reset(). This is
  /// consistent with previous implementation of utcTime() and localTime().
  bool needsToReset_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_DATECACHE_H
