/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

/// A DecoratedObject is a subclass of JSObject which owns a pointer to a native
/// C++ object, called its Decoration.  (This has nothing to do with
/// jsi::RuntimeDecoration or anything else in jsi/decorator.h.)
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
  /// If allocation fails, the GC declares an OOM.
  static PseudoHandle<DecoratedObject> create(
      Runtime *runtime,
      Handle<JSObject> parentHandle,
      std::unique_ptr<Decoration> decoration);

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

  using Super = JSObject;
  static ObjectVTable vt;
  static bool classof(const GCCell *cell) {
    return kindInRange(
        cell->getKind(),
        CellKind::DecoratedObjectKind_first,
        CellKind::DecoratedObjectKind_last);
  }

 protected:
  ~DecoratedObject() = default;

  DecoratedObject(
      Runtime *runtime,
      const ObjectVTable *vt,
      JSObject *parent,
      HiddenClass *clazz,
      std::unique_ptr<Decoration> decoration)
      : JSObject(runtime, &vt->base, parent, clazz),
        decoration_(std::move(decoration)) {}

  static void _finalizeImpl(GCCell *cell, GC *);
  static size_t _mallocSizeImpl(GCCell *cell);

 private:
#ifdef HERMESVM_SERIALIZE
  explicit DecoratedObject(Deserializer &d);

  friend void DecoratedObjectSerialize(Serializer &s, const GCCell *cell);
  friend void DecoratedObjectDeserialize(Deserializer &d, CellKind kind);
#endif

  std::unique_ptr<Decoration> decoration_;
};

} // namespace vm
} // namespace hermes
#endif
