/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s -typed | %FileCheck --match-full-lines -check-prefix MD %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines -check-prefix SM %s
// RUN: %shermes -exec %s -typed -script | %FileCheck --match-full-lines -check-prefix SM %s

// Running this in script mode will print undefined.
// In 'module'-like mode, it will print object.
print(typeof exports);
//MD-LABEL: object
//SM-LABEL: undefined
