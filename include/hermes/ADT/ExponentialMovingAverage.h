/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ADT_EXPONENTIALMOVINGAVERAGE_H
#define HERMES_ADT_EXPONENTIALMOVINGAVERAGE_H

#include <cassert>

namespace hermes {

/// An exponentially-weighted moving average. Newer values have more priority
/// than older values, which eventually decrease to have no impact on the
/// average. The weight is specified in the constructor.
class ExponentialMovingAverage {
  double weight_;
  double avg_;

 public:
  /// \param weight The weight factor between 0 and 1 to apply to the newer
  /// values. A higher weight means new values matter more than old values.
  /// \param init The starting point for the average.
  explicit ExponentialMovingAverage(double weight, double init)
      : weight_(weight), avg_(init) {
    assert(weight >= 0 && weight <= 1 && "Weight must be between 0 and 1");
  }

  /// Implicitly casts to its average value for ease-of-use.
  operator double() const {
    return avg_;
  }

  void update(double value) {
    avg_ = avg_ * (1 - weight_) + weight_ * value;
  }
};

} // namespace hermes

#endif
