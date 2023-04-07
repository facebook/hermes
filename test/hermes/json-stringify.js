/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print('json.stringify');
//CHECK-LABEL: json.stringify

print(JSON.stringify('𝌆'));
// CHECK-NEXT "𝌆"
print(JSON.stringify('\uD834\uDF06'));
// CHECK-NEXT "𝌆"
print(JSON.stringify('\uD834'));
// CHECK-NEXT "\\ud834"
print(JSON.stringify('\uDF06'));
// CHECK-NEXT "\\udf06"
print(JSON.stringify('\uDF06\uD834'));
// CHECK-NEXT "\\udf06\\ud834"
print(JSON.stringify('\uDEAD'));
// CHECK-NEXT "\\udead"
print(JSON.stringify('\uD834\uD834\uDF06'));
// CHECK-NEXT "\\ud834𝌆"
print(JSON.stringify('\uD834a'));
// CHECK-NEXT "\\ud834a"
print(JSON.stringify('\uD834\u0400'));
// CHECK-NEXT "\\ud834Ѐ"
print(JSON.stringify("\ud7ff"));
// CHECK-NEXT "\ud7ff"
print(JSON.stringify("\ud800"));
// CHECK-NEXT "\\ud800"
print(JSON.stringify("\ud937"));
// CHECK-NEXT "\\ud937"
print(JSON.stringify("\uda20"));
// CHECK-NEXT "\\uda20"
print(JSON.stringify("\udbff"));
// CHECK-NEXT "\\udbff"
print(JSON.stringify("\udc00"));
// CHECK-NEXT "\\udc00"
print(JSON.stringify("\udddd"));
// CHECK-NEXT "\\udddd"
print(JSON.stringify("\udeaf"));
// CHECK-NEXT "\\udeaf"
print(JSON.stringify("\udfff"));
// CHECK-NEXT "\\udfff"
print(JSON.stringify("\ue000"));
// CHECK-NEXT "\ue000"
print(JSON.stringify("\ud7ffa"));
// CHECK-NEXT "\ud7ffa"
print(JSON.stringify("\ud800a"));
// CHECK-NEXT "\\ud800a"
print(JSON.stringify("\ud937a"));
// CHECK-NEXT "\\ud937a"
print(JSON.stringify("\uda20a"));
// CHECK-NEXT "\\uda20a"
print(JSON.stringify("\udbffa"));
// CHECK-NEXT "\\udbffa"
print(JSON.stringify("\udc00a"));
// CHECK-NEXT "\\udc00a"
print(JSON.stringify("\udddda"));
// CHECK-NEXT "\\udddda"
print(JSON.stringify("\udeafa"));
// CHECK-NEXT "\\udeafa"
print(JSON.stringify("\udfffa"));
// CHECK-NEXT "\\udfffa"
print(JSON.stringify("\ue000a"));
// CHECK-NEXT "\ue000a"
print(JSON.stringify("\ud7ff\ud800"));
// CHECK-NEXT "\ud7ff\\ud800"
print(JSON.stringify("\ud800\ud800"));
// CHECK-NEXT "\\ud800\\ud800"
print(JSON.stringify("\ud937\ud800"));
// CHECK-NEXT "\\ud937\\ud800"
print(JSON.stringify("\uda20\ud800"));
// CHECK-NEXT "\\uda20\\ud800"
print(JSON.stringify("\udbff\ud800"));
// CHECK-NEXT "\\udbff\\ud800"
print(JSON.stringify("\udc00\ud800"));
// CHECK-NEXT "\\udc00\\ud800"
print(JSON.stringify("\udddd\ud800"));
// CHECK-NEXT "\\udddd\\ud800"
print(JSON.stringify("\udeaf\ud800"));
// CHECK-NEXT "\\udeaf\\ud800"
print(JSON.stringify("\udfff\ud800"));
// CHECK-NEXT "\\udfff\\ud800"
print(JSON.stringify("\ue000\ud800"));
// CHECK-NEXT "\ue000\\ud800"
print(JSON.stringify("\ud7ff\udc00"));
// CHECK-NEXT "\ud7ff\\udc00"
print(JSON.stringify("\ud800\udc00"));
// CHECK-NEXT "\ud800\udc00"
print(JSON.stringify("\ud937\udc00"));
// CHECK-NEXT "\ud937\udc00"
print(JSON.stringify("\uda20\udc00"));
// CHECK-NEXT "\uda20\udc00"
print(JSON.stringify("\udbff\udc00"));
// CHECK-NEXT "\udbff\udc00"
print(JSON.stringify("\udc00\udc00"));
// CHECK-NEXT "\\udc00\\udc00"
print(JSON.stringify("\udddd\udc00"));
// CHECK-NEXT "\\udddd\\udc00"
print(JSON.stringify("\udeaf\udc00"));
// CHECK-NEXT "\\udeaf\\udc00"
print(JSON.stringify("\udfff\udc00"));
// CHECK-NEXT "\\udfff\\udc00"
print(JSON.stringify("\ue000\udc00"));
// CHECK-NEXT "\ue000\\udc00"
