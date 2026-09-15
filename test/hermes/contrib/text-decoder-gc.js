/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// REQUIRES: contrib_extensions
// RUN: %hermes -O -target=HBC -gc-init-heap=4M -gc-max-heap=16M %s | %FileCheck --match-full-lines %s
"use strict";

print('TextDecoder GC');
// CHECK-LABEL: TextDecoder GC

// A 'buffer' getter can hand back an ArrayBuffer that nothing else references.
// decode() has to keep it alive while it builds the result string: that
// allocation can trigger a young-gen collection, whose finalizer frees the
// storage being decoded. Under ASan the stale read aborts; otherwise this
// detects only the corruption it produces.
function decodeUnrooted(decoder, size) {
  // Fill the young generation so the result string allocation collects.
  var junk = null;
  for (var i = 0; i < 500; ++i) {
    junk = {a: i, b: [i, i + 1]};
  }
  return decoder.decode({
    get buffer() {
      var bytes = new Uint8Array(size);
      bytes.fill(0x41);
      return bytes.buffer;
    },
    byteOffset: 0,
    byteLength: size,
  });
}

function isDecodedAs(str, size) {
  return (
    str.length === size &&
    str.charCodeAt(0) === 0x41 &&
    str.charCodeAt(size >> 1) === 0x41 &&
    str.charCodeAt(size - 1) === 0x41
  );
}

// Both decoders take an ASCII fast path that builds the result string straight
// from the buffer: utf-8 and any single-byte encoding.
var utf8Decoder = new TextDecoder('utf-8');
var singleByteDecoder = new TextDecoder('windows-1252');
var corrupted = 0;
for (var n = 0; n < 256; ++n) {
  // Sweep buffer sizes so a collection lands inside the window.
  var size = 1024 + (n % 128) * 1024;
  if (!isDecodedAs(decodeUnrooted(utf8Decoder, size), size)) {
    ++corrupted;
  }
  if (!isDecodedAs(decodeUnrooted(singleByteDecoder, size), size)) {
    ++corrupted;
  }
}
print('corrupted:', corrupted);
// CHECK-NEXT: corrupted: 0
