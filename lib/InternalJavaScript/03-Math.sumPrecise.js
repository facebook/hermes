/* @nolint *//**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

  // Math.sumPrecise polyfill from TC39 proposal
  // https://github.com/tc39/proposal-math-sum

  /*
  BSD 3-Clause License

  Copyright (c) 2024 Kevin Gibbons

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Algorithm based on:
  - Shewchuk's algorithm for exact floating point addition
    https://www-2.cs.cmu.edu/afs/cs/project/quake/public/papers/robust-arithmetic.ps
  - Python's math.fsum implementation
    https://github.com/python/cpython/blob/48dfd74a9db9d4aa9c6f23b4a67b461e5d977173/Modules/mathmodule.c#L1359-L1474
    Portions derived from Python - Copyright (c) 2001 Python Software Foundation; All Rights Reserved
  - Adapted to handle overflow via an additional "biased" partial, representing 2**1024 times its actual value
  */

  Math.sumPrecise = function sumPrecise(iterable) {
    // Type check - ensure iterable has Symbol.iterator
    if (!iterable || typeof iterable[Symbol.iterator] !== 'function') {
      throw new TypeError('Math.sumPrecise requires an iterable');
    }

    // exponent 11111111110, significand all 1s
    const MAX_DOUBLE = 1.79769313486231570815e+308; // i.e. (2**1024 - 2**(1023 - 52))

    // exponent 11111111110, significand all 1s except final 0
    const PENULTIMATE_DOUBLE = 1.79769313486231550856e+308; // i.e. (2**1024 - 2 * 2**(1023 - 52))

    // exponent 11111001010, significand all 0s
    const MAX_ULP = MAX_DOUBLE - PENULTIMATE_DOUBLE; // 1.99584030953471981166e+292, i.e. 2**(1023 - 52)

    // prerequisite: Math.abs(x) >= Math.abs(y)
    function twosum(x, y) {
      let hi = x + y;
      let lo = y - (hi - x);
      return { hi, lo };
    }

    let partials = [];

    let overflow = 0; // conceptually 2**1024 times this value; the final partial

    // for purposes of the polyfill we're going to ignore closing the iterator, sorry
    let iterator = iterable[Symbol.iterator]();
    let next = iterator.next.bind(iterator);

    // in C this would be done using a goto
    function drainNonFiniteValue(current) {
      while (true) {
        let { done, value } = next();
        if (done) {
          return current;
        }
        if (!Number.isFinite(value)) {
          // summing any distinct two of the three non-finite values gives NaN
          // summing any one of them with itself gives itself
          if (!Object.is(value, current)) {
            current = NaN;
          }
        }
      }
    }

    // handle list of -0 special case
    while (true) {
      let { done, value } = next();
      if (done) {
        return -0;
      }
      if (!Object.is(value, -0)) {
        // Check if value is not a number type
        if (typeof value !== 'number') {
          throw new TypeError('Math.sumPrecise expects numbers');
        }
        if (!Number.isFinite(value)) {
          return drainNonFiniteValue(value);
        }
        partials.push(value);
        break;
      }
    }

    // main loop
    while (true) {
      let { done, value } = next();
      if (done) {
        break;
      }
      // Check if value is not a number type (reject BigInt, objects, etc)
      if (typeof value !== 'number') {
        throw new TypeError('Math.sumPrecise expects numbers');
      }
      let x = Number(value);
      if (!Number.isFinite(x)) {
        return drainNonFiniteValue(x);
      }

      // we're updating partials in place, but it is maybe easier to understand if you think of it as making a new copy
      let actuallyUsedPartials = 0;
      // let newPartials = [];
      for (let y of partials) {
        if (Math.abs(x) < Math.abs(y)) {
          [x, y] = [y, x];
        }
        let { hi, lo } = twosum(x, y);
        if (Math.abs(hi) === Infinity) {
          let sign = hi === Infinity ? 1 : -1;
          overflow += sign;
          if (Math.abs(overflow) >= 2**53) {
            throw new RangeError('overflow');
          }

          x = (x - sign * 2**1023) - sign * 2**1023;
          if (Math.abs(x) < Math.abs(y)) {
            [x, y] = [y, x];
          }
          ({ hi, lo } = twosum(x, y));
        }
        if (lo !== 0) {
          partials[actuallyUsedPartials] = lo;
          ++actuallyUsedPartials;
          // newPartials.push(lo);
        }
        x = hi;
      }
      partials.length = actuallyUsedPartials;
      // assert.deepStrictEqual(partials, newPartials)
      // partials = newPartials

      if (x !== 0) {
        partials.push(x);
      }
    }

    // compute the exact sum of partials, stopping once we lose precision
    let n = partials.length - 1;
    let hi = 0;
    let lo = 0;

    if (overflow !== 0) {
      let next = n >= 0 ? partials[n] : 0;
      --n;
      if (Math.abs(overflow) > 1 || (overflow > 0 && next > 0 || overflow < 0 && next < 0)) {
        return overflow > 0 ? Infinity : -Infinity;
      }
      // here we actually have to do the arithmetic
      // drop a factor of 2 so we can do it without overflow
      // assert(Math.abs(overflow) === 1)
      ({ hi, lo } = twosum(overflow * 2**1023, next / 2));
      lo *= 2;
      if (Math.abs(2 * hi) === Infinity) {
        // stupid edge case: rounding to the maximum value
        // MAX_DOUBLE has a 1 in the last place of its significand, so if we subtract exactly half a ULP from 2**1024, the result rounds away from it (i.e. to infinity) under ties-to-even
        // but if the next partial has the opposite sign of the current value, we need to round towards MAX_DOUBLE instead
        // this is the same as the "handle rounding" case below, but there's only one potentially-finite case we need to worry about, so we just hardcode that one
        if (hi > 0) {
          if (hi === 2**1023 && lo === -(MAX_ULP / 2) && n >= 0 && partials[n] < 0) {
            return MAX_DOUBLE;
          }
          return Infinity;
        } else {
          if (hi === -(2**1023) && lo === (MAX_ULP / 2) && n >= 0 && partials[n] > 0) {
            return -MAX_DOUBLE;
          }
          return -Infinity;
        }
      }
      if (lo !== 0) {
        partials[n + 1] = lo;
        ++n;
        lo = 0;
      }
      hi *= 2;
    }

    while (n >= 0) {
      let x = hi;
      let y = partials[n];
      --n;
      // assert: Math.abs(x) > Math.abs(y)
      ({ hi, lo } = twosum(x, y));
      if (lo !== 0) {
        break;
      }
    }

    // handle rounding
    // when the roundoff error is exactly half of the ULP for the result, we need to check one more partial to know which way to round
    if (n >= 0 && ((lo < 0.0 && partials[n] < 0.0) ||
                  (lo > 0.0 && partials[n] > 0.0))) {
      let y = lo * 2.0;
      let x = hi + y;
      let yr = x - hi;
      if (y === yr) {
        hi = x;
      }
    }

    return hi;
  };
