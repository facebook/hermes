/**
 * BSD 3-Clause License
 *
 * Portions Copyright (c) Meta Platforms, Inc. and affiliates.
 * Copyright (c) 2024 Kevin Gibbons
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *  may be used to endorse or promote products derived from this software
 *  without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

try { Math.sumPrecise([{}]); } catch(e) { print(e.name); }
//CHECK: TypeError

try { Math.sumPrecise([0n]); } catch(e) { print(e.name); }
//CHECK-NEXT: TypeError

var coercions = 0;
var objectWithValueOf = {
  valueOf: function() {
    ++coercions;
    throw new Error("valueOf should not be called");
  },
  toString: function() {
    ++coercions;
    throw new Error("toString should not be called");
  }
};

try { Math.sumPrecise([objectWithValueOf]); } catch(e) { print(e.name); }
//CHECK-NEXT: TypeError
print(coercions);
//CHECK-NEXT: 0

try { Math.sumPrecise([objectWithValueOf, NaN]); } catch(e) { print(e.name); }
//CHECK-NEXT: TypeError
print(coercions);
//CHECK-NEXT: 0

try { Math.sumPrecise([NaN, objectWithValueOf]); } catch(e) { print(e.name); }
//CHECK-NEXT: TypeError
print(coercions);
//CHECK-NEXT: 0

try { Math.sumPrecise([-Infinity, Infinity, objectWithValueOf]); } catch(e) { print(e.name); }
//CHECK-NEXT: TypeError
print(coercions);
//CHECK-NEXT: 0

var nextCalls = 0;
var returnCalls = 0;
var iterator = {
  next: function () {
    ++nextCalls;
    return { done: false, value: objectWithValueOf };
  },
  return: function () {
    ++returnCalls;
    return {};
  }
};
var iterable = {
  [Symbol.iterator]: function () {
    return iterator;
  }
};

try { Math.sumPrecise(iterable); } catch(e) { print(e.name); }
//CHECK-NEXT: TypeError
print(coercions, nextCalls, returnCalls);
//CHECK-NEXT: 0 1 1
