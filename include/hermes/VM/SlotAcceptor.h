/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SLOTACCEPTOR_H
#define HERMES_VM_SLOTACCEPTOR_H

#include "hermes/VM/HermesValue.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

class GCPointerBase;
class WeakRefBase;
class WeakRootBase;
class GCCell;

/// SlotAcceptor is an interface to be implemented by acceptors of objects in
/// the heap.
/// An acceptor should accept all of the pointers and other markable fields in
/// an object, and tell the GC that they exist, updating if necessary.
/// (The accept methods should make no assumptions about the address of the
/// slot; in some cases, an adaptor class may store a pointer in a local, call
/// the acceptor on the local, and then write the local back into the
/// pointer.  For example, if the pointer is in compressed form.)
/// This is used by a visitor, see \c SlotVisitor.
struct SlotAcceptor {
  virtual ~SlotAcceptor() = default;
  virtual void accept(GCPointerBase &ptr) = 0;
  virtual void accept(GCHermesValue &hv) = 0;
  virtual void accept(GCSymbolID sym) = 0;
};

/// Weak references are typically slower to find, and need to be done separately
/// from normal references.
struct WeakRefAcceptor {
  virtual ~WeakRefAcceptor() = default;
  virtual void accept(WeakRefBase &wr) = 0;
};

struct RootSectionAcceptor {
  virtual ~RootSectionAcceptor() = default;

  enum class Section {
#define ROOT_SECTION(name) name,
#include "hermes/VM/RootSections.def"
    NumSections,
    // Sentinel value to be used to represent an invalid section.
    InvalidSection,
  };

  virtual void beginRootSection(Section section) {}
  virtual void endRootSection() {}
};

struct RootAcceptor : public RootSectionAcceptor {
  virtual void accept(GCCell *&ptr) = 0;
  virtual void accept(PinnedHermesValue &hv) = 0;
  virtual void accept(RootSymbolID sym) = 0;

  /// When we want to call an acceptor on "raw" root pointers of
  /// some JSObject subtype T, this method does the necessary
  /// reinterpret_cast to allow us to call the "GCCell *&" accept
  /// method above.
  template <typename T>
  void acceptPtr(T *&ptr) {
    accept(reinterpret_cast<GCCell *&>(ptr));
  }
};

struct RootAndSlotAcceptor : public RootAcceptor, public SlotAcceptor {
  using RootAcceptor::accept;
  using SlotAcceptor::accept;
};

struct RootAndSlotAcceptorWithNames : public RootAndSlotAcceptor {
  void accept(GCCell *&ptr) final {
    accept(ptr, nullptr);
  }
  virtual void accept(GCCell *&ptr, const char *name) = 0;

  void accept(PinnedHermesValue &hv) final {
    accept(hv, nullptr);
  }
  virtual void accept(PinnedHermesValue &hv, const char *name) = 0;

  void accept(RootSymbolID sym) final {
    accept(sym, nullptr);
  }
  virtual void accept(RootSymbolID sym, const char *name) = 0;

  using RootAndSlotAcceptor::acceptPtr;
  template <typename T>
  void acceptPtr(T *&ptr, const char *name) {
    accept(reinterpret_cast<GCCell *&>(ptr), name);
  }

  void accept(GCPointerBase &ptr) final {
    accept(ptr, nullptr);
  }
  virtual void accept(GCPointerBase &ptr, const char *name) = 0;

  void accept(GCHermesValue &hv) final {
    accept(hv, nullptr);
  }
  virtual void accept(GCHermesValue &hv, const char *name) = 0;

  void accept(GCSymbolID sym) final {
    accept(sym, nullptr);
  }
  virtual void accept(GCSymbolID sym, const char *name) = 0;

  /// Initiate the callback if this acceptor is part of heap snapshots.
  virtual void provideSnapshot(
      const std::function<void(HeapSnapshot &)> &func) {}
};

struct WeakRootAcceptor : public WeakRefAcceptor, RootSectionAcceptor {
  ~WeakRootAcceptor() override = default;

  /// NOTE: This is called acceptWeak in order to avoid clashing with accept
  /// from SlotAcceptor, for classes that inherit from both.
  virtual void acceptWeak(WeakRootBase &ptr) = 0;
};

template <typename Acceptor>
struct DroppingAcceptor final : public RootAndSlotAcceptorWithNames {
  static_assert(
      std::is_base_of<RootAndSlotAcceptor, Acceptor>::value,
      "Can only use this with a subclass of RootAndSlotAcceptor");
  Acceptor &acceptor;

  explicit DroppingAcceptor(Acceptor &acceptor) : acceptor(acceptor) {}

  using RootAndSlotAcceptorWithNames::accept;

  void accept(GCCell *&ptr, const char *) override {
    acceptor.accept(ptr);
  }

  void accept(GCPointerBase &ptr, const char *) override {
    acceptor.accept(ptr);
  }

  void accept(PinnedHermesValue &hv, const char *) override {
    acceptor.accept(hv);
  }

  void accept(GCHermesValue &hv, const char *) override {
    acceptor.accept(hv);
  }

  void accept(RootSymbolID sym, const char *) override {
    acceptor.accept(sym);
  }

  void accept(GCSymbolID sym, const char *) override {
    acceptor.accept(sym);
  }
};

} // namespace vm
} // namespace hermes

#endif
