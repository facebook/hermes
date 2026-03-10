/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

(function() {
  var ARRAY_SIZE = 10000;
  var ITERATIONS = 2000;

  var int8Sort = Int8Array.prototype.sort;
  var uint8Sort = Uint8Array.prototype.sort;
  var uint8cSort = Uint8ClampedArray.prototype.sort;
  var int16Sort = Int16Array.prototype.sort;
  var uint16Sort = Uint16Array.prototype.sort;
  var int32Sort = Int32Array.prototype.sort;
  var uint32Sort = Uint32Array.prototype.sort;
  var bigint64sort = BigInt64Array.prototype.sort;
  var biguint64sort = BigUint64Array.prototype.sort;
  var float32Sort = Float32Array.prototype.sort;
  var float64Sort = Float64Array.prototype.sort;

  // Generate a template of random bytes to fill arrays from.
  var templateBuf = new ArrayBuffer(ARRAY_SIZE * 8); // big enough for Float64
  var templateBytes = new Uint8Array(templateBuf);
  for (var i = 0; i < templateBytes.length; i++) {
    templateBytes[i] = (Math.random() * 256) | 0;
  }

  function bench(name, TypedArrayCtor, sortFn) {
    // Create the source array from our random template buffer.
    var source = new TypedArrayCtor(
      templateBuf.slice(0, ARRAY_SIZE * TypedArrayCtor.BYTES_PER_ELEMENT)
    );
    var arr = new TypedArrayCtor(ARRAY_SIZE);
    var start = Date.now();
    for (var i = 0; i < ITERATIONS; i++) {
      // Reset to unsorted data.
      arr.set(source);
      // Sort using the captured function.
      sortFn.call(arr);
    }
    var elapsed = Date.now() - start;
    print(name + ": " + elapsed + " ms");
  }

  print("TypedArray.prototype.sort benchmark");
  print("Array size: " + ARRAY_SIZE + ", iterations: " + ITERATIONS);
  print("--------------------------------------------");

  bench("Int8Array       ", Int8Array, int8Sort);
  bench("Uint8Array      ", Uint8Array, uint8Sort);
  bench("Uint8ClampedArr ", Uint8ClampedArray, uint8cSort);
  bench("Int16Array      ", Int16Array, int16Sort);
  bench("Uint16Array     ", Uint16Array, uint16Sort);
  bench("Int32Array      ", Int32Array, int32Sort);
  bench("Uint32Array     ", Uint32Array, uint32Sort);
  bench("BigInt64Array   ", BigInt64Array, bigint64sort);
  bench("BigUint64Array  ", BigUint64Array, biguint64sort);
  bench("Float32Array    ", Float32Array, float32Sort);
  bench("Float64Array    ", Float64Array, float64Sort);
})();
