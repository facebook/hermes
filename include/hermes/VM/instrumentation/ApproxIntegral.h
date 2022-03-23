/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_INSTRUMENTATION_APPROXINTEGRAL_H
#define HERMES_VM_INSTRUMENTATION_APPROXINTEGRAL_H

#include <cassert>
#include <cstdint>

namespace hermes {
namespace vm {

/// Accumulator for approximating the integral under a line, in two dimensions,
/// using the trapezoidal rule.  Given a function f(x), and samples {(x, y)_i},
/// where:
///
///     0 <= i <= N
///     x_{i-1} <= x_i for 0 <  i <= N
///     y_i = f(x_i)   for 0 <= i <= N
///
/// Approximates
///
///     \int_{x_0}^{x_N} f(x) dx
///
/// By splitting the area under the line into N trapezoids, where the left and
/// right edges of trapezoid T (0 <= T < N) are verticals at x_T and x_{T+1}
/// respectively, which meet the x axis on one side, and the line described by
/// f(x) on the other. The accuracy of the approximation improves with the
/// number of samples.
///
/// Note that the first co-ordinate is fixed at the origin -- (0, 0).
class ApproxIntegral {
 public:
  /// Extend the line by a new sample
  ///
  /// \p x The position of the sample on the x-axis.
  /// \p y The position of the sample on the y-axis.  Assumed to be the value of
  ///     the function -- f --, being sampled, at x (i.e. f(x)).
  ///
  /// \pre x must be greater than all previous x co-ordinates pushed into this
  ///     accumulator.
  inline void push(int64_t x, int64_t y);

  /// \return the approximation of the area under the samples witnessed so far.
  inline int64_t area() const;

 private:
  /// Previous (x, y) co-ordinates pushed into the accumulator.
  int64_t xLast_{0};
  int64_t yLast_{0};

  /// Double the approximated integral accumulated so far.  The total is stored
  /// doubled to avoid accumulating integer rounding error from halving each
  /// summand individually.
  int64_t total_{0};
};

void ApproxIntegral::push(int64_t x, int64_t y) {
  assert(x >= xLast_ && "Points must be pushed in x-axis order.");

  total_ += (x - xLast_) * (y + yLast_);
  xLast_ = x;
  yLast_ = y;
}

int64_t ApproxIntegral::area() const {
  return total_ / 2;
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_INSTRUMENTATION_APPROXINTEGRAL_H
