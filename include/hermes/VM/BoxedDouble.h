/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_BOXEDDOUBLE_H
#define HERMES_VM_BOXEDDOUBLE_H

#include "hermes/VM/Runtime.h"

namespace hermes {
namespace vm {

/// A fixed size GCCell used to store a single immutable double. This is used by
/// SmallHermesValue to store numbers that cannot be represented as small
/// integers.
class BoxedDouble final : public GCCell {
  friend void BoxedDoubleBuildMeta(const GCCell *cell, Metadata::Builder &mb);

  static const VTable vt;

  double value_;

 public:
#ifdef HERMESVM_SERIALIZE
  explicit BoxedDouble(Deserializer &d);

  friend void BoxedDoubleSerialize(Serializer &s, const GCCell *cell);
  friend void BoxedDoubleDeserialize(Deserializer &d, CellKind kind);
#endif

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::BoxedDoubleKind;
  }

  static BoxedDouble *create(double d, Runtime *runtime) {
    return runtime->makeAFixed<BoxedDouble>(d, runtime);
  }

  BoxedDouble(double d, Runtime *runtime)
      : GCCell(&runtime->getHeap(), &vt), value_(d) {}

  double get() const {
    return value_;
  }
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_BOXEDDOUBLE_H
