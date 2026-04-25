/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// Benchmark for JSON.stringify and JSON.parse with string values.
// Exercises the string quoting/scanning hot paths where SIMD can
// skip over long runs of non-escape characters in bulk.

var start = Date.now();
(function () {
  var numIter = 200000;

  // Build strings with long runs of normal characters and occasional
  // escape characters to exercise both the SIMD bulk skip and the
  // scalar escape handling.
  var shortStr = 'hello world';
  var base = 'abcdefghijklmnopqrstuvwxyz0123456789';
  var longStr = '';
  for (var i = 0; i < 8; i++) {
    longStr += base;
  }
  // 296 chars, no escapes.

  var longStrWithEscapes = '';
  for (var i = 0; i < 8; i++) {
    // 35 normal chars then a quote and a backslash.
    longStrWithEscapes += 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789' + '"\\';
  }
  longStrWithEscapes = longStrWithEscapes.substring(0, 256);

  // An object with multiple string fields, typical of real payloads.
  var obj = {
    name: 'John Doe',
    address: '123 Main St, Anytown, USA 12345',
    bio: longStr.substring(0, 64),
    notes: longStrWithEscapes.substring(0, 64),
  };

  var sum = 0;
  for (var i = 0; i < numIter; i++) {
    // Stringify exercises quoteStringForJSON.
    var s1 = JSON.stringify(shortStr);
    var s2 = JSON.stringify(longStr);
    var s3 = JSON.stringify(longStrWithEscapes);
    var s4 = JSON.stringify(obj);
    sum += s1.length + s2.length + s3.length + s4.length;

    // Parse exercises JSONLexer::scanString.
    var p1 = JSON.parse(s1);
    var p2 = JSON.parse(s2);
    var p3 = JSON.parse(s3);
    var p4 = JSON.parse(s4);
    sum += p1.length + p2.length + p3.length + p4.notes.length;
  }

  print('done, sum =', sum);
})();
var end = Date.now();
print('Time: ' + (end - start));
