/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSLib/DateCache.h"

namespace hermes {
namespace vm {

double LocalTimeOffsetCache::getLocalTimeOffset(
    double timeMs,
    TimeType timeType) {
  if (needsToReset_) {
    reset();
  }

  if (timeType == TimeType::Utc) {
    // Out of allowed range by the spec, return NaN.
    if (timeMs < -TIME_RANGE_MS || timeMs > TIME_RANGE_MS)
      return std::numeric_limits<double>::quiet_NaN();

    return ltza_ + daylightSavingOffsetInMs(timeMs);
  }
  // To compute the DST offset, we need to use UTC time (as required by
  // daylightSavingOffsetInMs()). However, getting the exact UTC time is not
  // possible since that would be circular. Therefore, we approximate the UTC
  // time by subtracting the standard time adjustment and then subtracting an
  // additional hour to comply with the spec's requirements
  // (https://tc39.es/ecma262/#sec-utc-t).
  //
  // For example, imagine a transition to DST that goes from UTC+0 to UTC+1,
  // moving 00:00 to 01:00. Any time in the skipped hour gets mapped to a
  // UTC time before the transition when we subtract an hour (e.g., 00:30 ->
  // 23:30), which will correctly result in DST not being in effect.
  //
  // Similarly, during a transition from DST back to standard time, the hour
  // from 00:00 to 01:00 is repeated. A local time in the repeated hour
  // similarly gets mapped to a UTC time before the transition.
  //
  // Note that this will not work if the timezone offset has historical/future
  // changes (which generates a different ltza than the one obtained here).
  double guessUTC = timeMs - ltza_ - MS_PER_HOUR;
  if (guessUTC < -TIME_RANGE_MS || guessUTC > TIME_RANGE_MS)
    return std::numeric_limits<double>::quiet_NaN();
  return ltza_ + daylightSavingOffsetInMs(guessUTC);
}

int LocalTimeOffsetCache::computeDaylightSaving(int64_t utcTimeMs) {
  std::time_t t = utcTimeMs / MS_PER_SECOND;
  std::tm tm;
  int ltza;
#ifdef _WINDOWS
  auto err = ::localtime_s(&tm, &t);
  if (err) {
    return 0;
  }
  // It's not officially documented that whether Windows C API caches time zone,
  // but actual testing shows it does. So for now, we don't detect TZ changes
  // and reset the cache here. Otherwise, we have to call tzset() and
  // _get_timezone(), which is thread unsafe. And this behavior is the same as
  // on Linux.
#else
  std::tm *brokenTime = ::localtime_r(&t, &tm);
  if (!brokenTime) {
    return 0;
  }
  int dstOffset = tm.tm_isdst ? MS_PER_HOUR : 0;
  ltza = tm.tm_gmtoff * MS_PER_SECOND - dstOffset;
  // If ltza changes, we need to reset the cache.
  if (ltza != ltza_) {
    needsToReset_ = true;
  }
#endif
  return tm.tm_isdst ? MS_PER_HOUR : 0;
}

int LocalTimeOffsetCache::daylightSavingOffsetInMs(int64_t utcTimeMs) {
  if (needsToReset_) {
    reset();
  }

  // Some OS library calls don't work right for dates that cannot be represented
  // with int32_t. ES5.1 requires to map the time to a year with same
  // leap-year-ness and same starting day for the year. But for compatibility,
  // other engines, such as V8, use the actual year if it is in the range of
  // 1970..2037, which corresponds to the time range 0..kMaxEpochTimeInMs.
  if (utcTimeMs < 0 || utcTimeMs > kMaxEpochTimeInMs) {
    utcTimeMs =
        detail::equivalentTime(utcTimeMs / MS_PER_SECOND) * MS_PER_SECOND;
  }

  // Reset the counter to avoid overflow. Each call of this function may
  // increase epoch_ by more than 1 (conservatively smaller than 10), so we need
  // to subtract it from max value. In practice, this won't happen frequently
  // since most time we should see cache hit.
  if (LLVM_UNLIKELY(
          epoch_ >= std::numeric_limits<decltype(epoch_)>::max() - 10)) {
    reset();
  }

  // Cache hit.
  if (candidate_->include(utcTimeMs)) {
    candidate_->epoch = bumpEpoch();
    return candidate_->dstOffsetMs;
  }

  // Try to find cached intervals that happen before/after utcTimeMs.
  auto [before, after] = findBeforeAndAfterEntries(utcTimeMs);
  // Set candidate_ to before by default, and reassign it in case that the
  // after cache is hit or used.
  candidate_ = before;

  // No cached interval yet, compute a new one with utcTimeMs.
  if (before->isEmpty()) {
    int dstOffset = computeDaylightSaving(utcTimeMs);
    before->dstOffsetMs = dstOffset;
    before->startMs = utcTimeMs;
    before->endMs = utcTimeMs;
    before->epoch = bumpEpoch();
    return dstOffset;
  }

  // Hits in the cached interval.
  if (before->include(utcTimeMs)) {
    before->epoch = bumpEpoch();
    return before->dstOffsetMs;
  }

  // If utcTimeMs is larger than before->endMs + kDSTDeltaMs, we can't safely
  // extend before, because it could have more than one DST transition in the
  // interval. Instead, try if we can extend after (or recompute it).
  if ((utcTimeMs - kDSTDeltaMs) > before->endMs) {
    int dstOffset = computeDaylightSaving(utcTimeMs);
    extendOrRecomputeCacheEntry(after, utcTimeMs, dstOffset);
    // May help cache hit in subsequent calls (in case that the passed in time
    // values are adjacent).
    candidate_ = after;
    return dstOffset;
  }

  // Now, utcTimeMs is in the range of (before->endMs, before->endMs +
  // kDSTDeltaMs].

  before->epoch = bumpEpoch();
  int64_t newAfterStart = before->endMs < kMaxEpochTimeInMs - kDSTDeltaMs
      ? before->endMs + kDSTDeltaMs
      : kMaxEpochTimeInMs;
  // If after starts too late, extend it to newAfterStart or recompute it.
  if (newAfterStart < after->startMs) {
    int dstOffset = computeDaylightSaving(newAfterStart);
    extendOrRecomputeCacheEntry(after, newAfterStart, dstOffset);
  } else {
    after->epoch = bumpEpoch();
  }

  // Now after->startMs is in (before->endMs, before->endMs + kDSTDeltaMs].

  // If before and after have the same DST offset, merge them.
  if (before->dstOffsetMs == after->dstOffsetMs) {
    before->endMs = after->endMs;
    *after = DSTCacheEntry{};
    return before->dstOffsetMs;
  }

  // Binary search in (before->endMs, after->startMs] for DST transition
  // point. Note that after->startMs could be smaller than before->endMs
  // + kDSTDeltaMs, but that small interval has the same DST offset, so we
  // can ignore them in the below search.
  // Though 5 iterations should be enough to cover kDSTDeltaMs, if the
  // assumption of only one transition in kDSTDeltaMs no longer holds, we may
  // not be able to search the result. We'll stop the loop after 5 iterations
  // anyway.
  for (int i = 4; i >= 0; --i) {
    int delta = after->startMs - before->endMs;
    int64_t middle = before->endMs + delta / 2;
    int middleDstOffset = computeDaylightSaving(middle);
    if (before->dstOffsetMs == middleDstOffset) {
      before->endMs = middle;
      if (utcTimeMs <= before->endMs) {
        return middleDstOffset;
      }
    } else {
      assert(after->dstOffsetMs == middleDstOffset);
      after->startMs = middle;
      if (utcTimeMs >= after->startMs) {
        // May help cache hit in subsequent calls (in case that the passed in
        // time values are adjacent).
        candidate_ = after;
        return middleDstOffset;
      }
    }
  }

  // Fallthrough path of the binary search, just compute the DST for utcTimeMs.
  return computeDaylightSaving(utcTimeMs);
}

LocalTimeOffsetCache::DSTCacheEntry *
LocalTimeOffsetCache::leastRecentlyUsedExcept(const DSTCacheEntry *const skip) {
  DSTCacheEntry *result = nullptr;
  for (auto &cache : caches_) {
    if (&cache == skip)
      continue;
    if (!result || result->epoch > cache.epoch)
      result = &cache;
  }
  *result = DSTCacheEntry{};
  return result;
}

std::tuple<
    LocalTimeOffsetCache::DSTCacheEntry *,
    LocalTimeOffsetCache::DSTCacheEntry *>
LocalTimeOffsetCache::findBeforeAndAfterEntries(int64_t timeMs) {
  LocalTimeOffsetCache::DSTCacheEntry *before = nullptr;
  LocalTimeOffsetCache::DSTCacheEntry *after = nullptr;

  // `before` should start as late as possible, while `after` should end as
  // early as possible so that they're closest to timeMs.
  for (auto &cache : caches_) {
    if (cache.startMs <= timeMs) {
      if (!before || before->startMs < cache.startMs)
        before = &cache;
    } else if (timeMs < cache.endMs) {
      if (!after || after->endMs > cache.endMs)
        after = &cache;
    }
  }

  // None is found, reuse an empty cache for later computation.
  if (!before) {
    before = leastRecentlyUsedExcept(after);
  }
  if (!after) {
    after = leastRecentlyUsedExcept(before);
  }

  assert(
      before && after && before != after &&
      "`before` and `after` interval should be valid");
  assert(
      before->isEmpty() ||
      before->startMs <= timeMs &&
          "the start time of `before` must start on or before timeMs");
  assert(
      after->isEmpty() ||
      timeMs < after->startMs &&
          "The start time of `after` must start after timeMs");
  assert(
      before->isEmpty() || after->isEmpty() ||
      before->endMs < after->startMs &&
          "`before` interval must strictly start before `after` interval");

  return {before, after};
}

void LocalTimeOffsetCache::extendOrRecomputeCacheEntry(
    DSTCacheEntry *&entry,
    int64_t timeMs,
    int dstOffsetMs) {
  // It's safe to extend the interval if timeMs is in the checked range.
  if (entry->dstOffsetMs == dstOffsetMs &&
      entry->startMs - kDSTDeltaMs <= timeMs && timeMs <= entry->endMs) {
    entry->startMs = timeMs;
  } else {
    // Recompute the after cache using timeMs.
    if (!entry->isEmpty()) {
      entry = leastRecentlyUsedExcept(candidate_);
    }
    entry->startMs = timeMs;
    entry->endMs = timeMs;
    entry->dstOffsetMs = dstOffsetMs;
    entry->epoch = bumpEpoch();
  }
}

} // namespace vm
} // namespace hermes
