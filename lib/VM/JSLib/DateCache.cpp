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
  // daylightSavingTA()). However, getting the exact UTC time is not possible
  // since that would be circular. Therefore, we approximate the UTC time by
  // subtracting the standard time adjustment and then subtracting an additional
  // hour to comply with the spec's requirements as noted in the doc-comment.
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

int LocalTimeOffsetCache::computeDaylightSaving(int64_t timeMs) {
  std::time_t local = timeMs / MS_PER_SECOND;
  std::tm tm;
  int ltza;
#ifdef _WINDOWS
  auto err = ::localtime_s(&tm, &local);
  if (err) {
    return 0;
  }
  // It's not officially documented that whether Windows C API caches time zone,
  // but actual testing shows it does. So for now, we don't detect TZ changes
  // and reset the cache here. Otherwise, we have to call tzset() and
  // _get_timezone(), which is thread unsafe. And this behavior is the same as
  // on Linux.
#else
  std::tm *brokenTime = ::localtime_r(&local, &tm);
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

int LocalTimeOffsetCache::daylightSavingOffsetInMs(int64_t timeMs) {
  if (needsToReset_) {
    reset();
  }

  // Some OS library calls don't work right for dates that cannot be represented
  // with int32_t. ES5.1 requires to map the time to a year with same
  // leap-year-ness and same starting day for the year. But for compatibility,
  // other engines, such as V8, use the actual year if it is in the range of
  // 1970..2037, which corresponds to the time range 0..kMaxEpochTimeInMs.
  if (timeMs < 0 || timeMs > kMaxEpochTimeInMs) {
    timeMs = detail::equivalentTime(timeMs / MS_PER_SECOND) * MS_PER_SECOND;
  }

  // Reset the counter to avoid overflow.
  if (LLVM_UNLIKELY(
          epoch_ >= std::numeric_limits<decltype(epoch_)>::max() - 10)) {
    reset();
  }

  // Cache hit.
  if (before_->startMs <= timeMs && timeMs <= before_->endMs) {
    before_->epoch = bumpEpoch();
    return before_->dstOffsetMs;
  }

  // Try to find cached intervals that happen before/after timeMs.
  findBeforeAndAfterEntries(timeMs);

  assert(before_->isEmpty() || before_->startMs <= timeMs);
  assert(after_->isEmpty() || timeMs < after_->startMs);

  // No cached interval yet, compute a new one with timeMs.
  if (before_->isEmpty()) {
    int dstOffset = computeDaylightSaving(timeMs);
    before_->dstOffsetMs = dstOffset;
    before_->startMs = timeMs;
    before_->endMs = timeMs;
    before_->epoch = bumpEpoch();
    return dstOffset;
  }

  // Hits in the cached interval.
  if (timeMs <= before_->endMs) {
    before_->epoch = bumpEpoch();
    return before_->dstOffsetMs;
  }

  // If timeMs is larger than before_->endMs + kDSTDeltaMs, we can't safely
  // extend before_, because it could have more than one DST transition in the
  // interval. Instead, try if we can extend after_ (or recompute it).
  if ((timeMs - kDSTDeltaMs) > before_->endMs) {
    int dstOffset = computeDaylightSaving(timeMs);
    extendOrRecomputeAfterCache(timeMs, dstOffset);
    // May help cache hit in subsequent calls (in case that the passed in time
    // values are adjacent).
    std::swap(before_, after_);
    return dstOffset;
  }

  // Now, timeMs is in the range of (before_->endMs, before_->endMs +
  // kDSTDeltaMs].

  before_->epoch = bumpEpoch();
  int64_t newAfterStart = before_->endMs < kMaxEpochTimeInMs - kDSTDeltaMs
      ? before_->endMs + kDSTDeltaMs
      : kMaxEpochTimeInMs;
  // If after_ starts too late, extend it to newAfterStart or recompute it.
  if (newAfterStart < after_->startMs) {
    int dstOffset = computeDaylightSaving(newAfterStart);
    extendOrRecomputeAfterCache(newAfterStart, dstOffset);
  } else {
    after_->epoch = bumpEpoch();
  }

  // Now after_->startMs is in (before_->endMs, before_->endMs + kDSTDeltaMs].
  if (before_->dstOffsetMs == after_->dstOffsetMs) {
    before_->endMs = after_->endMs;
    *after_ = DSTCacheEntry{};
    return before_->dstOffsetMs;
  }

  // Binary search in (before_->endMs, after_->startMs] for DST transition
  // point. Note that after_->startMs could be smaller than before_->endMs
  // + kDSTDeltaMs, but that small interval has the same DST offset, so we
  // can ignore them in the below search.
  // Though 5 iterations should be enough to cover kDSTDeltaMs, if the
  // assumption of only one transition in kDSTDeltaMs no longer holds, we may
  // not be able to search the result. We'll stop the loop after 5 iterations
  // anyway.
  for (int i = 4; i >= 0; --i) {
    int delta = after_->startMs - before_->endMs;
    int64_t middle = !i ? timeMs : before_->endMs + delta / 2;
    int dstOffset = computeDaylightSaving(middle);
    if (before_->dstOffsetMs == dstOffset) {
      before_->endMs = middle;
      if (timeMs <= before_->endMs) {
        return dstOffset;
      }
    } else {
      assert(after_->dstOffsetMs == dstOffset);
      after_->startMs = middle;
      if (timeMs >= after_->startMs) {
        std::swap(before_, after_);
        return dstOffset;
      }
    }
  }

  // Fallthrough path of the binary search, just compute the DST for timeMs.
  return computeDaylightSaving(timeMs);
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

void LocalTimeOffsetCache::findBeforeAndAfterEntries(int64_t timeMs) {
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
    before = before_->isEmpty() ? before_ : leastRecentlyUsedExcept(after);
  }
  if (!after) {
    after = after_->isEmpty() && before != after_
        ? after_
        : leastRecentlyUsedExcept(before);
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

  before_ = before;
  after_ = after;
}

void LocalTimeOffsetCache::extendOrRecomputeAfterCache(
    int64_t timeMs,
    int dstOffsetMs) {
  // It's safe to extend the interval if timeMs is in the checked range.
  if (after_->dstOffsetMs == dstOffsetMs &&
      after_->startMs - kDSTDeltaMs <= timeMs && timeMs <= after_->endMs) {
    after_->startMs = timeMs;
  } else {
    // Recompute the after_ cache using timeMs.
    if (!after_->isEmpty()) {
      after_ = leastRecentlyUsedExcept(before_);
    }
    after_->startMs = timeMs;
    after_->endMs = timeMs;
    after_->dstOffsetMs = dstOffsetMs;
    after_->epoch = bumpEpoch();
  }
}

} // namespace vm
} // namespace hermes
