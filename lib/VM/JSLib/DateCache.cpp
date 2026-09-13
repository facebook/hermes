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

    return localOffsetInMs((int64_t)timeMs);
  }
  // To compute the total offset, we need to use UTC time (as required by
  // localOffsetInMs()). However, getting the exact UTC time is not
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
  // Note that the guess uses ltza_, the *current* standard offset. If the
  // standard offset was different at the converted date (e.g. Europe/Kyiv was
  // UTC+3 until 1991 and is UTC+2 now), the guess may be off by more than an
  // hour; the probed total offset is still correct as long as no offset
  // transition falls between the guess and the true UTC time.
  double guessUTC = timeMs - ltza_ - MS_PER_HOUR;
  if (guessUTC < -TIME_RANGE_MS || guessUTC > TIME_RANGE_MS)
    return std::numeric_limits<double>::quiet_NaN();
  return localOffsetInMs((int64_t)guessUTC);
}

int LocalTimeOffsetCache::computeLocalOffset(int64_t utcTimeMs) {
  std::time_t t = utcTimeMs / MS_PER_SECOND;
  std::tm tm;
#ifdef _WINDOWS
  auto err = ::localtime_s(&tm, &t);
  if (err) {
    return 0;
  }
  // The Windows C API does not provide tm_gmtoff, and it applies the current
  // DST rules to all dates, so the standard offset cannot vary with time: the
  // total offset is the cached standard offset plus the DST offset.
  // It's not officially documented that whether Windows C API caches time zone,
  // but actual testing shows it does. So for now, we don't detect TZ changes
  // and reset the cache here. Otherwise, we have to call tzset() and
  // _get_timezone(), which is thread unsafe. And this behavior is the same as
  // on Linux.
  return (int)(ltza_ + (tm.tm_isdst ? MS_PER_HOUR : 0));
#else
  std::tm *brokenTime = ::localtime_r(&t, &tm);
  if (!brokenTime) {
    return 0;
  }
  // tm_gmtoff is the total offset (standard offset + DST) at utcTimeMs. Cache
  // the total instead of just the DST offset: the standard offset itself can
  // change over history (e.g. Europe/Kyiv was UTC+3 until 1991 and is UTC+2
  // now), so adding a single cached standard offset to a cached DST offset
  // would give wrong results for such dates.
  int totalOffset = (int)(tm.tm_gmtoff * MS_PER_SECOND);
  int stdOffset = totalOffset - (tm.tm_isdst ? MS_PER_HOUR : 0);
  if (stdOffset != ltza_) {
    // The standard offset at utcTimeMs differs from the cached one. This is
    // expected for historical dates in time zones whose standard offset
    // changed; the cached total offset is still correct for them. Only reset
    // if the standard offset at the *current* time changed too, which means
    // the TZ environment itself was updated (possible on MacOS, where the C
    // library does not cache time zone information).
    if ((int64_t)localTZA() != ltza_) {
      needsToReset_ = true;
    }
  }
  return totalOffset;
#endif
}

int LocalTimeOffsetCache::localOffsetInMs(int64_t utcTimeMs) {
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
    return candidate_->offsetMs;
  }

  // Try to find cached intervals that happen before/after utcTimeMs.
  auto [before, after] = findBeforeAndAfterEntries(utcTimeMs);
  // Set candidate_ to before by default, and reassign it in case that the
  // after cache is hit or used.
  candidate_ = before;

  // No cached interval yet, compute a new one with utcTimeMs.
  if (before->isEmpty()) {
    int offset = computeLocalOffset(utcTimeMs);
    before->offsetMs = offset;
    before->startMs = utcTimeMs;
    before->endMs = utcTimeMs;
    before->epoch = bumpEpoch();
    return offset;
  }

  // Hits in the cached interval.
  if (before->include(utcTimeMs)) {
    before->epoch = bumpEpoch();
    return before->offsetMs;
  }

  // If utcTimeMs is larger than before->endMs + kDSTDeltaMs, we can't safely
  // extend before, because it could have more than one offset transition in
  // the interval. Instead, try if we can extend after (or recompute it).
  if ((utcTimeMs - kDSTDeltaMs) > before->endMs) {
    int offset = computeLocalOffset(utcTimeMs);
    extendOrRecomputeCacheEntry(after, utcTimeMs, offset);
    // May help cache hit in subsequent calls (in case that the passed in time
    // values are adjacent).
    candidate_ = after;
    return offset;
  }

  // Now, utcTimeMs is in the range of (before->endMs, before->endMs +
  // kDSTDeltaMs].

  before->epoch = bumpEpoch();
  // If before->endMs gets too large, we need to make sure it won't overflow
  // kMaxEpochTimeInMs after extending it.
  int64_t newAfterStart = before->endMs < kMaxEpochTimeInMs - kDSTDeltaMs
      ? before->endMs + kDSTDeltaMs
      : kMaxEpochTimeInMs;
  // We need to handle two cases here:
  // 1. If after starts too late, recompute it or extend it to newAfterStart.
  // 2. If after is empty, its startMs would be kMaxEpochTimeInMs. And if
  // newAfterStart is also capped to kMaxEpochTimeInMs, we would need to
  // recompute the after entry.
  if (newAfterStart <= after->startMs) {
    int offset = computeLocalOffset(newAfterStart);
    extendOrRecomputeCacheEntry(after, newAfterStart, offset);
  } else {
    after->epoch = bumpEpoch();
  }

  // Now after->startMs is in (before->endMs, before->endMs + kDSTDeltaMs].

  // If before and after have the same offset, merge them.
  if (before->offsetMs == after->offsetMs) {
    before->endMs = after->endMs;
    *after = DSTCacheEntry{};
    return before->offsetMs;
  }

  // Binary search in (before->endMs, after->startMs] for the offset transition
  // point. Note that after->startMs could be smaller than before->endMs
  // + kDSTDeltaMs, but that small interval has the same offset, so we
  // can ignore them in the below search.
  // Though 5 iterations should be enough to cover kDSTDeltaMs, if the
  // assumption of only one transition in kDSTDeltaMs no longer holds, we may
  // not be able to search the result. We'll stop the loop after 5 iterations
  // anyway.
  for (int i = 4; i >= 0; --i) {
    int64_t delta = after->startMs - before->endMs;
    int64_t middle = before->endMs + delta / 2;
    int middleOffset = computeLocalOffset(middle);
    if (before->offsetMs == middleOffset) {
      before->endMs = middle;
      if (utcTimeMs <= before->endMs) {
        return middleOffset;
      }
    } else {
      assert(after->offsetMs == middleOffset);
      after->startMs = middle;
      if (utcTimeMs >= after->startMs) {
        // May help cache hit in subsequent calls (in case that the passed in
        // time values are adjacent).
        candidate_ = after;
        return middleOffset;
      }
    }
  }

  // Fallthrough path of the binary search, just compute the offset for
  // utcTimeMs.
  return computeLocalOffset(utcTimeMs);
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
    int offsetMs) {
  // It's safe to extend the interval if timeMs is in the checked range.
  if (entry->offsetMs == offsetMs &&
      entry->startMs - kDSTDeltaMs <= timeMs && timeMs <= entry->endMs) {
    entry->startMs = timeMs;
  } else {
    // Recompute the after cache using timeMs.
    if (!entry->isEmpty()) {
      entry = leastRecentlyUsedExcept(candidate_);
    }
    entry->startMs = timeMs;
    entry->endMs = timeMs;
    entry->offsetMs = offsetMs;
    entry->epoch = bumpEpoch();
  }
}

} // namespace vm
} // namespace hermes
