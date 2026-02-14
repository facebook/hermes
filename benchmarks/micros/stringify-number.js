/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Benchmark for JSON.stringify of numbers.

function benchmark() {
  let iterations = 150000;
  let stringify = JSON.stringify;
  let log = typeof print === "undefined" ? console.log : print;

  function iteration() {
    // Case 1: NaN -> "NaN"
    stringify(NaN);

    // Case 2: +0 or -0 -> "0"
    stringify(0);
    stringify(-0);

    // Case 3: Negative numbers (recursively call toString(-x))
    stringify(-42);
    stringify(-3.14159);
    stringify(-123.456);
    stringify(-0.0001);
    stringify(-1e10);

    // Infinity
    stringify(Infinity);
    stringify(-Infinity);

    // Case 5a: Large integers (n >= k, no decimal point needed)
    stringify(1);
    stringify(42);
    stringify(123);
    stringify(1000);
    stringify(12345);
    stringify(1000000);
    stringify(999999999);

    // Case 5b: Regular decimals (0 < n < k, decimal point in the middle)
    stringify(0.1);
    stringify(0.5);
    stringify(1.5);
    stringify(3.14159);
    stringify(123.456);
    stringify(99.99);

    // Case 5c: Small decimals (n <= 0, leading "0." with zeros)
    stringify(0.01);
    stringify(0.001);
    stringify(0.0001);
    stringify(0.00001);
    stringify(0.000001);

    // Case 6: Scientific notation (radix 10, n outside [-5, 21])
    // k=1 cases (single digit mantissa)
    stringify(1e21);
    stringify(1e22);
    stringify(2e25);
    stringify(5e30);
    stringify(1e-7);
    stringify(1e-10);
    stringify(3e-15);

    // k>1 cases (multiple digit mantissa)
    stringify(1.23e21);
    stringify(9.87654e25);
    stringify(1.5e30);
    stringify(2.71828e22);
    stringify(1.23e-7);
    stringify(9.87e-10);
    stringify(3.14159e-15);

    // Edge cases for the n in [-5, 21] boundary
    stringify(1e-5);
    stringify(1e-6);
    stringify(1e20);
    stringify(1e21);

    // More varied mantissa lengths
    stringify(1.234567890123456e10);
    stringify(9.999999999999999e15);
    stringify(1.111111111111111e-10);
  }

  let start = Date.now();

  for (let i = 0; i < iterations; i++) {
    iteration();
  }

  let elapsed = Date.now() - start;
  log(`Time: ${elapsed}`);
}

benchmark();
