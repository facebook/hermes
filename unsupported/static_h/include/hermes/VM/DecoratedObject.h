/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_DECORATEDOBJECT_H
#define HERMES_VM_DECORATEDOBJECT_H

#include "hermes/VM/JSObject.h"

#include <memory>

namespace hermes {
namespace vm {

/// A DecoratedObject is a subclass of JSObject which owns a pointer to a
/// native C++ object, called its Decoration.  (This has nothing to do with
/// jsi::RuntimeDecoration or anything else in jsi/decorator.h.)  It can also
/// include additional internal slots, as in OrdinaryObjectCreate's
/// additionalInternalSlotsList).
class DecoratedObject : public JSObject {
 public:
  struct Decoration {
    /// The destructor is called when the decorated object is finalized.
    /// Do not attempt to manipulate the Runtime from within this destructor.
    virtual ~Decoration() = default;

    /// If implemented, this returns the size of the decoration, for memory
    /// usage reporting. The default implementation returns sizeof(*this).
    virtual size_t getMallocSize() const;
  };

  /// Allocate a DecoratedObject with the given prototype and decoration.

  /// \param additionalSlotCount internal slots to reserve within the
  /// object.  Only a small number of slots are available; this value
  /// cannot be greater than InternalProperty::NumAnonymousInternalProperties -
  /// numOverlaps, which is currently 3.
  /// If allocation fails, the GC declares an OOM.
  static PseudoHandle<DecoratedObject> create(
      Runtime &runtime,
      Handle<JSObject> parentHandle,
      std::unique_ptr<Decoration> decoration,
      unsigned int additionalSlotCount = 0);

  /// Access the decoration.
  Decoration *getDecoration() {
    return decoration_.get();
  }

  /// Access the decoration.
  const Decoration *getDecoration() const {
    return decoration_.get();
  }

  /// Set the decoration to a new value \p value.
  void setDecoration(std::unique_ptr<Decoration> val) {
    decoration_ = std::move(val);
  }

  /// \return the value in an additional slot.
  /// \param index must be less than the \c additionalSlotCount passed to
  /// the create method.
  static SmallHermesValue getAdditionalSlotValue(
      DecoratedObject *self,
      Runtime &runtime,
      unsigned index) {
    return JSObject::getInternalProperty(
        self, runtime, numOverlapSlots<DecoratedObject>() + index);
  }

  /// Set the value in an additional slot.
  /// \param index must be less than the \c additionalSlotCount passed to
  /// the create method.
  static void setAdditionalSlotValue(
      DecoratedObject *self,
      Runtime &runtime,
      unsigned index,
      SmallHermesValue value) {
    JSObject::setInternalProperty(
        self, runtime, numOverlapSlots<DecoratedObject>() + index, value);
  }

  using Super = JSObject;
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::DecoratedObjectKind;
  }
  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::DecoratedObjectKind_first,
        CellKind::DecoratedObjectKind_last);
  }

 public:
  ~DecoratedObject() = default;

  DecoratedObject(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      std::unique_ptr<Decoration> decoration)
      : JSObject(runtime, *parent, *clazz),
        decoration_(std::move(decoration)) {}

 protected:
  static void _finalizeImpl(GCCell *cell, GC &);
  static size_t _mallocSizeImpl(GCCell *cell);

 private:
  std::unique_ptr<Decoration> decoration_;
};

} // namespace vm
} // namespace hermes
#endif
