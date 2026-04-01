/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck %s
// RUN: %shermes -exec %s | %FileCheck %s

// Tests for the TC39 "Uint8Array to/from Base64 and Hex" proposal.

'use strict';

// === toHex ===
print("toHex");
// CHECK-LABEL: toHex
print("[" + new Uint8Array([]).toHex() + "]");
// CHECK-NEXT: []
print(new Uint8Array([72, 101, 108, 108, 111]).toHex());
// CHECK-NEXT: 48656c6c6f
print(new Uint8Array([0, 1, 254, 255]).toHex());
// CHECK-NEXT: 0001feff

// === toBase64 ===
print("toBase64");
// CHECK-LABEL: toBase64
print("[" + new Uint8Array([]).toBase64() + "]");
// CHECK-NEXT: []
print(new Uint8Array([72, 101, 108, 108, 111]).toBase64());
// CHECK-NEXT: SGVsbG8=
print(new Uint8Array([1, 2, 3]).toBase64());
// CHECK-NEXT: AQID
print(new Uint8Array([1, 2]).toBase64());
// CHECK-NEXT: AQI=
print(new Uint8Array([1]).toBase64());
// CHECK-NEXT: AQ==

// toBase64 with omitPadding
print("toBase64 omitPadding");
// CHECK-LABEL: toBase64 omitPadding
print(new Uint8Array([1]).toBase64({omitPadding: true}));
// CHECK-NEXT: AQ
print(new Uint8Array([1, 2]).toBase64({omitPadding: true}));
// CHECK-NEXT: AQI

// toBase64 with base64url alphabet
print("toBase64 base64url");
// CHECK-LABEL: toBase64 base64url
print(new Uint8Array([251, 255, 254]).toBase64());
// CHECK-NEXT: +//+
print(new Uint8Array([251, 255, 254]).toBase64({alphabet: "base64url"}));
// CHECK-NEXT: -__-

// === fromHex ===
print("fromHex");
// CHECK-LABEL: fromHex
var r = Uint8Array.fromHex("48656c6c6f");
print(r.length, r[0], r[1], r[2], r[3], r[4]);
// CHECK-NEXT: 5 72 101 108 108 111
r = Uint8Array.fromHex("");
print(r.length);
// CHECK-NEXT: 0
r = Uint8Array.fromHex("0001feff");
print(r.length, r[0], r[1], r[2], r[3]);
// CHECK-NEXT: 4 0 1 254 255

// fromHex case insensitive
r = Uint8Array.fromHex("ABCDEF");
print(r[0], r[1], r[2]);
// CHECK-NEXT: 171 205 239

// fromHex error cases
print("fromHex errors");
// CHECK-LABEL: fromHex errors
try { Uint8Array.fromHex("abc"); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: SyntaxError
try { Uint8Array.fromHex("gg"); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: SyntaxError
try { Uint8Array.fromHex(123); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: TypeError

// === fromBase64 ===
print("fromBase64");
// CHECK-LABEL: fromBase64
r = Uint8Array.fromBase64("SGVsbG8=");
print(r.length, r[0], r[1], r[2], r[3], r[4]);
// CHECK-NEXT: 5 72 101 108 108 111
r = Uint8Array.fromBase64("");
print(r.length);
// CHECK-NEXT: 0
r = Uint8Array.fromBase64("AQID");
print(r.length, r[0], r[1], r[2]);
// CHECK-NEXT: 3 1 2 3

// fromBase64 with whitespace
r = Uint8Array.fromBase64("S G Vs b G 8 =");
print(r.length, r[0], r[1], r[2], r[3], r[4]);
// CHECK-NEXT: 5 72 101 108 108 111

// fromBase64 with base64url
r = Uint8Array.fromBase64("-__-", {alphabet: "base64url"});
print(r.length, r[0], r[1], r[2]);
// CHECK-NEXT: 3 251 255 254

// fromBase64 lastChunkHandling: "loose" (default)
r = Uint8Array.fromBase64("AQ");
print(r.length, r[0]);
// CHECK-NEXT: 1 1
r = Uint8Array.fromBase64("AQI");
print(r.length, r[0], r[1]);
// CHECK-NEXT: 2 1 2

// fromBase64 lastChunkHandling: "strict"
print("fromBase64 strict");
// CHECK-LABEL: fromBase64 strict
try { Uint8Array.fromBase64("AQ", {lastChunkHandling: "strict"}); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: SyntaxError
r = Uint8Array.fromBase64("AQ==", {lastChunkHandling: "strict"});
print(r.length, r[0]);
// CHECK-NEXT: 1 1

// fromBase64 lastChunkHandling: "stop-before-partial"
print("fromBase64 stop-before-partial");
// CHECK-LABEL: fromBase64 stop-before-partial
r = Uint8Array.fromBase64("AQIDBA", {lastChunkHandling: "stop-before-partial"});
print(r.length, r[0], r[1], r[2]);
// CHECK-NEXT: 3 1 2 3

// fromBase64 error cases
print("fromBase64 errors");
// CHECK-LABEL: fromBase64 errors
try { Uint8Array.fromBase64(123); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: TypeError
try { Uint8Array.fromBase64("!!!"); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: SyntaxError

// === setFromHex ===
print("setFromHex");
// CHECK-LABEL: setFromHex
var arr = new Uint8Array(5);
var result = arr.setFromHex("48656c6c6f");
print(result.read, result.written, arr[0], arr[1], arr[2], arr[3], arr[4]);
// CHECK-NEXT: 10 5 72 101 108 108 111

// setFromHex partial (target too small)
arr = new Uint8Array(2);
result = arr.setFromHex("48656c6c6f");
print(result.read, result.written, arr[0], arr[1]);
// CHECK-NEXT: 4 2 72 101

// setFromHex error cases
print("setFromHex errors");
// CHECK-LABEL: setFromHex errors
try { arr.setFromHex("abc"); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: SyntaxError
try { Uint8Array.prototype.setFromHex.call([], "ab"); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: TypeError

// === setFromBase64 ===
print("setFromBase64");
// CHECK-LABEL: setFromBase64
arr = new Uint8Array(5);
result = arr.setFromBase64("SGVsbG8=");
print(result.read, result.written, arr[0], arr[1], arr[2], arr[3], arr[4]);
// CHECK-NEXT: 8 5 72 101 108 108 111

// setFromBase64 partial: target has 3 bytes, decodes one full chunk.
arr = new Uint8Array(3);
result = arr.setFromBase64("SGVsbG8=");
print(result.read, result.written, arr[0], arr[1], arr[2]);
// CHECK-NEXT: 4 3 72 101 108

// setFromBase64 maxLength boundary: target has 2 byte capacity,
// spec stops before accumulating 4th char since remaining=2 < 3.
arr = new Uint8Array(2);
result = arr.setFromBase64("AQID");
print(result.read, result.written, arr[0], arr[1]);
// CHECK-NEXT: 0 0 0 0

// setFromBase64 maxLength boundary: target has 1 byte capacity,
// spec stops before accumulating 3rd char since remaining=1 < 2.
arr = new Uint8Array(1);
result = arr.setFromBase64("AQID");
print(result.read, result.written, arr[0]);
// CHECK-NEXT: 0 0 0

// setFromBase64 error cases
print("setFromBase64 errors");
// CHECK-LABEL: setFromBase64 errors
try { Uint8Array.prototype.setFromBase64.call([], "AQ=="); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: TypeError
try { arr.setFromBase64(123); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: TypeError

// Method length properties
print("lengths");
// CHECK-LABEL: lengths
print(Uint8Array.prototype.toBase64.length);
// CHECK-NEXT: 0
print(Uint8Array.prototype.toHex.length);
// CHECK-NEXT: 0
print(Uint8Array.fromBase64.length);
// CHECK-NEXT: 1
print(Uint8Array.fromHex.length);
// CHECK-NEXT: 1
print(Uint8Array.prototype.setFromBase64.length);
// CHECK-NEXT: 1
print(Uint8Array.prototype.setFromHex.length);
// CHECK-NEXT: 1

// Methods should not exist on other TypedArray prototypes
print("not on Int8Array");
// CHECK-LABEL: not on Int8Array
print(typeof Int8Array.prototype.toBase64);
// CHECK-NEXT: undefined
print(typeof Int8Array.prototype.toHex);
// CHECK-NEXT: undefined
print(typeof Int8Array.fromBase64);
// CHECK-NEXT: undefined

// Non-zero padding bits: loose ignores, strict rejects
print("non-zero padding bits");
// CHECK-LABEL: non-zero padding bits
r = Uint8Array.fromBase64("ZXhhZh==");
print(r[0], r[1], r[2], r[3]);
// CHECK-NEXT: 101 120 97 102
r = Uint8Array.fromBase64("ZXhhZh==", {lastChunkHandling: "loose"});
print(r[0], r[1], r[2], r[3]);
// CHECK-NEXT: 101 120 97 102
try { Uint8Array.fromBase64("ZXhhZh==", {lastChunkHandling: "strict"}); } catch(e) { print(e.constructor.name); }
// CHECK-NEXT: SyntaxError

// Partial padding with stop-before-partial returns earlier bytes
print("partial padding sbp");
// CHECK-LABEL: partial padding sbp
r = Uint8Array.fromBase64("ZXhhZg=", {lastChunkHandling: "stop-before-partial"});
print(r.length, r[0], r[1], r[2]);
// CHECK-NEXT: 3 101 120 97

// Excess padding always errors
try { Uint8Array.fromBase64("ZXhhZg==="); } catch(e) { print(e.constructor.name); }
// CHECK-NEXT: SyntaxError
try { Uint8Array.fromBase64("ZXhhZg===", {lastChunkHandling: "stop-before-partial"}); } catch(e) { print(e.constructor.name); }
// CHECK-NEXT: SyntaxError

// GetOptionsObject: non-object, non-undefined options should throw TypeError
print("options validation");
// CHECK-LABEL: options validation
try { new Uint8Array([1]).toBase64("base64url"); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: TypeError
try { Uint8Array.fromBase64("AQ==", 42); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: TypeError

// setFromBase64 writes partial bytes before throwing on error
print("write before error");
// CHECK-LABEL: write before error
arr = new Uint8Array(4);
try { arr.setFromBase64("AQID!!!"); } catch (e) { print(e.constructor.name, arr[0], arr[1], arr[2]); }
// CHECK-NEXT: SyntaxError 1 2 3

print("DONE");
// CHECK-LABEL: DONE
