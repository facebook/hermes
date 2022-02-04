/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_STATSACCUMULATOR_H
#define HERMES_SUPPORT_STATSACCUMULATOR_H

#include <algorithm>
#include <cmath>

namespace hermes {

/// Gathers summary statistics for a sequence of samples for a
/// statistic of type T.  The statistic type T must be convertible to
/// double.  SumT is provided for cases where the sum of the samples
/// might overflow the type T.  T must be convertible to SumT, and
/// SumT must be convertible to double.
template <typename T, typename SumT = T>
class StatsAccumulator {
 public:
  StatsAccumulator() = default;

  /// Update the summary stats with the addition of a new \p value.
  inline void record(T value) {
    if (n_ == 0) {
      min_ = value;
      max_ = value;
    } else {
      min_ = std::min(min_, value);
      max_ = std::max(max_, value);
    }
    n_++;
    sum_ += value;
    double valD = static_cast<double>(value);
    sumOfSquares_ += valD * valD;
  }

  /// Accessors

  /// \return the number of samples recorded.
  inline unsigned count() const {
    return n_;
  }

  /// \return the minimum of the samples recorded.
  inline T min() const {
    return min_;
  }

  /// \return the maximum of the samples recorded.
  inline T max() const {
    return max_;
  }

  /// \return the sum of the samples recorded.
  inline SumT sum() const {
    return sum_;
  }

  /// Returns the average of the recorded samples.
  inline double average() const {
    return n_ == 0 ? 0.0 : static_cast<double>(sum_) / n_;
  }

  /// \return the sum of the squares of the samples recorded.
  inline double sumOfSquares() const {
    return sumOfSquares_;
  }

  /// Returns the standard deviation of the recorded samples.
  inline double stddev() const;

 private:
  /// Number of samples recorded.
  unsigned n_{0};
  /// Sum of the samples.
  SumT sum_{0};
  /// Minimum sample.
  T min_{0};
  /// Maximum sample.
  T max_{0};
  /// Sum of the squares of the durations (necessary for standard
  /// devation).  We assume that because of the squaring, it may grow
  /// large, so always use double for this value.
  double sumOfSquares_{0.0};
};

template <typename T, typename SumT>
inline double StatsAccumulator<T, SumT>::stddev() const {
  if (n_ == 0) {
    return 0.0;
  }
  double avg = average();
  // See, e.g.,
  // https://math.stackexchange.com/questions/198336/how-to-calculate-standard-deviation-with-streaming-inputs
  // for an explanation.
  double variance = (sumOfSquares_ / n_) - (avg * avg);
  return std::sqrt(variance);
}

} // namespace hermes

#endif // HERMES_SUPPORT_STATSACCUMULATOR_H
