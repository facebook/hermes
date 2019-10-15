/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_RUNTIME_LIBHERMES_H
#define HERMES_RUNTIME_LIBHERMES_H

// This is a list of built-in symbols declared by the HermesVM runtime.
// clang-format off
const char libhermes[] =
"var Object;"
"var Function;"
"var Array;"
"var String;"
"var Boolean;"
"var Number;"
"var Math;"
"var Date;"
"var RegExp;"
"var Error;"
"var JSON;"
"var HermesInternal;"
"var Symbol;"
"var Map;"
"var Set;"
"var WeakMap;"
"var WeakSet;"
""
"var Infinity;"
"var NaN;"
"var undefined;"
""
"function Error() {}"
"function EvalError() {}"
"function RangeError() {}"
"function ReferenceError() {}"
"function SyntaxError() {}"
"function TypeError() {}"
"function URIError() {}"
"function ArrayBuffer() {}"
"function DataView() {}"
#define TYPED_ARRAY(name, type) \
  "function " #name "Array() {}"
#include "hermes/VM/TypedArrays.def"
""
"function print() {}"
"function eval() {}"
"function parseInt() {}"
"function parseFloat() {}"
"function isNaN() {}"
"function isFinite() {}"
"function escape()  {}"
"function unescape()  {}"
"function decodeURI() {}"
"function decodeURIComponent() {}"
"function encodeURI() {}"
"function encodeURIComponent() {}"
"function gc() {}";
// clang-format on
#endif
