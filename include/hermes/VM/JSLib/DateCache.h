/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSLIB_DATECACHE_H
#define HERMES_VM_JSLIB_DATECACHE_H

#include "hermes/VM/JSLib/DateUtil.h"

namespace hermes {
namespace vm {

/// Type of time represented by an epoch.
enum class TimeType : int8_t {
  Local,
  Utc,
};

/// Cache local time offset (including daylight saving offset).
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
/// which is considered an "empty" interval, here t_max is the maximum epoch
/// time supported by some OS time APIs. Initialize candidate_ to point to the
/// first interval in the array. And constant kDSTDeltaMs is the maximum
/// time range that can have at most one DST transition. Each entry also has an
/// epoch value that is used to find out the least recently used entry, but we
/// omit the operations on them in this description.
/// 2. Given a UTC time point t, check if it's included in candidate_, if yes,
/// return the DST offset of candidate_. Otherwise, go to step 3.
/// 3. Search the cache array and try to find two intervals:
///    before: [s_before, e_before] where s_before <= t and as close as
///    possible.
//     after: [s_after, e_after] where t < s_after and e_after ends as early as
///    possible.
///    If one interval is not found, let it point to an empty interval in the
///    cache array (if no empty interval in it, reset the least recently used
///    one to empty and use it). Assign before to candidate_.
/// 4. Check if the the before interval is empty, if yes, compute the DST of t,
///    and set its interval to [t, t]. Otherwise, go to step 5.
/// 5. Check if t is included in the new non-empty before interval, if yes,
///    return its DST. Otherwise, go to step 6.
/// 6. If t > R_new, where R_new = before.end + kDSTDeltaMs, compute the DST
///    offset for t and call extendOrRecomputeCacheEntry(after, t, offset) (this
///    function is described later) to update after, assign after to candidate_,
///    return offset. Otherwise, go to step 7.
/// 7. If t <= R_new < after.start, compute the DST offset for t and call
///    extendOrRecomputeCacheEntry(after, t, offset) to update after.
/// 8. If before.offset == after.offset, merge after to before and
///    reset after, then return the offset. Otherwise, go to step 9.
/// 9. At this step, there must be one DST transition in interval
///    (before.end, after.start], compute DST of t and do binary search to
///    find a time point that has the same offset as t, and extend before or
///    after to it. In the end, return the offset (if after is hit, assign it
///    to candidate_).
/// 10. Given a new UTC time point, repeat step 2 - 9.
///
/// Algorithm for extendOrRecomputeAfterCache(entry, t, offset):
/// 1. If offset == entry.offset and entry.start-kDSTDeltaMs <= t <= entry.end,
///    let entry.start = t and return.
/// 2. If entry is not empty, scan every entry in the cache array (except
///    candidate_), find out the least recently used one, reset it to empty.
/// 3. Assign offset to entry and update its interval to be [t, t].
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
    candidate_ = caches_.data();
    epoch_ = 0;
    needsToReset_ = false;
  }

  /// Compute local timezone offset (DST included).
  /// \param timeMs time in milliseconds.
  /// \param timeType whether \p timeMs is UTC or local time.
  double getLocalTimeOffset(double timeMs, TimeType timeType);

  /// \param utcTimeMs UTC epoch in milliseconds.
  /// \return Daylight saving offset at time \p utcTimeMs.
  int daylightSavingOffsetInMs(int64_t utcTimeMs);

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

    /// \return True if \p utcTimeMs is included in this interval.
    bool include(int64_t utcTimeMs) const {
      return startMs <= utcTimeMs && utcTimeMs <= endMs;
    }
  };

  /// Compute the DST offset at UTC time \p timeMs.
  /// Note that this may update needsToReset_ if it detects a different
  /// standard offset than the cached one.
  int computeDaylightSaving(int64_t utcTimeMs);

  /// Increase the epoch counter and return it.
  int bumpEpoch() {
    ++epoch_;
    return epoch_;
  }

  /// Find the least recently used DST cache and reuse it.
  /// \param skip do not scan this cache. Technically, it can be nullptr, which
  /// means we don't skip any entry in the cache array. But currently we always
  /// passed in an meaningful entry pointer (e.g., candidate_).
  /// \return an empty DST cache. This should never return nullptr.
  DSTCacheEntry *leastRecentlyUsedExcept(const DSTCacheEntry *const skip);

  /// Scan the DST caches to find a cached interval starts at or before \p
  /// timeMs (as late as possible) and an interval ends after \p timeMs (as
  /// early as possible). If none is found, reuse an empty cache. Return the
  /// found two cache entries.
  std::tuple<DSTCacheEntry *, DSTCacheEntry *> findBeforeAndAfterEntries(
      int64_t timeMs);

  /// If entry->dstOffsetMs == \p dstOffsetMs and \p timeMs is included in
  /// [entry->startMs - kDSTDeltaMs, entry->endMs], extend entry to
  /// [timeMs, entry->endMs].
  /// Otherwise, let \p entry point to the least recently used cache entry
  /// (except the candidate_ cache) and update its interval to be [timeMs,
  /// timeMs], and its DST offset to be \p dstOffsetMs.
  void extendOrRecomputeCacheEntry(
      DSTCacheEntry *&entry,
      int64_t timeMs,
      int dstOffsetMs);

  /// Integer counter used to find least recently used cache.
  int epoch_;
  /// Point to one entry in the caches_ array, and this invariant always
  /// holds: candidate_->startMs <= t <= candidate_->endMs, where t is the last
  /// seen time point. It likely will cover subsequent time points.
  DSTCacheEntry *candidate_;
  /// A list of cached intervals computed for previously seen time points.
  /// In the beginning, each interval is initialized to empty. When every
  /// interval is non-empty and we need a new empty one, reset the least
  /// recently used one (by comparing the epoch value) to empty.
  std::array<DSTCacheEntry, kCacheSize> caches_;
  /// The standard local timezone offset (without DST offset).
  int64_t ltza_;
  /// Whether needs to reset the cache and ltza_.
  /// We don't do reset in the middle of daylightSavingOffsetInMs() (essentially
  /// before any call to computeDaylightSaving() that will detect TZ changes)
  /// because it may cause that function never return if another thread is
  /// keeping updating TZ. But that means we may return incorrect result before
  /// reset(). This is consistent with previous implementation of utcTime() and
  /// localTime().
  bool needsToReset_;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSLIB_DATECACHE_H
