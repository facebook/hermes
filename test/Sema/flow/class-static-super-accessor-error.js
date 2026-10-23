/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Intended location: test/Sema/flow/class-static-super-accessor-error.js
// RUN: (! %shermes -typed -dump-sema -ferror-limit=0 %s 2>&1) | %FileCheckOrRegen %s

'use strict';

class AccessorErrorBase {
  static backing: number = 0;

  @Hermes.final
  static get readOnly(): number {
    return AccessorErrorBase.backing;
  }

  @Hermes.final
  static set writeOnly(value: number) {
    AccessorErrorBase.backing = value;
  }
}

class AccessorErrorChild extends AccessorErrorBase {
  static assignGetter(): void {
    super.readOnly = 1;
  }

  static readSetter(): void {
    super.writeOnly;
  }

  static compoundSetter(): void {
    super.writeOnly += 1;
  }

  static wrongValue(): void {
    super.writeOnly = 'bad';
  }
}

// Check the diagnostics at the relevant expressions; additional diagnostics
// may follow the invalid compound operation on the setter-only property.
// CHECK: error: ft: cannot assign to getter-only property
// CHECK-NEXT: {{ *}}super.readOnly = 1;
// CHECK: error: ft: cannot read setter-only property
// CHECK-NEXT: {{ *}}super.writeOnly;
// CHECK: error: ft: cannot read setter-only property
// CHECK-NEXT: {{ *}}super.writeOnly += 1;
// CHECK: error: ft: incompatible assignment type:
// CHECK-NEXT: {{ *}}super.writeOnly = 'bad';
