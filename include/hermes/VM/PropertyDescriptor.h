/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_PROPERTYDESCRIPTOR_H
#define HERMES_VM_PROPERTYDESCRIPTOR_H

#include <cstdint>

#include "hermes/VM/Deserializer.h"

namespace hermes {
namespace vm {

/// @name PropertyDescriptor
/// @{

/// Flags associated with a single property descriptor.
union PropertyFlags {
  struct {
    /// If set, the entire structure is invalid. This must only be used when we
    /// want to distinguish between having an instance of \c PropertyFlags and
    /// not having one. It should never be stored in a \c PropertyDescriptor.
    /// This flag should never be set manually - instead an "invalid" instance
    /// should be obtained by calling the  \c invalid() factory method.  This
    /// flag is a little more special than the others, and should remain first
    /// in the struct.
    uint32_t invalidFlags : 1;
    /// The property is enumerable.
    uint32_t enumerable : 1;
    /// The property can be written to, if it is a data descriptor.
    uint32_t writable : 1;
    /// The property can be configured.
    uint32_t configurable : 1;
    /// The property is an accessor: it has an optional getter and setter.
    uint32_t accessor : 1;
    /// The property is stored in "indexed" storage, which is separate from
    /// normal named property storage.
    uint32_t indexed : 1;
    /// A regular data descriptor with an internal setter handler which is
    /// invoked on writes.
    uint32_t internalSetter : 1;
    /// This flag is only used in the hidden class transition table. It
    /// indicates that the property wasn't added - only its flags were updated.
    uint32_t flagsTransition : 1;
    /// This property indicates that the object is a HostObject
    /// property managed by C++.  This flag should only be used as a
    /// marker for certain temporary descriptors synthesized by
    /// get*Descriptor methods, and never set in descriptors stored
    /// persistently.
    uint32_t hostObject : 1;
    /// This property is a builtin method or object, and it could be
    /// accessed by the CallBuiltin instruction. The property is made read-only
    /// and is not allowed to be overriden.
    uint32_t staticBuiltin : 1;
  };

  uint32_t _flags;

  /// Clear all flags on construction.
  PropertyFlags() {
    _flags = 0;
  }

  /// Fast constructor used by Deserializer. Read data from the stream directly.
  PropertyFlags(Deserializer &d) {
    d.readData(&_flags, sizeof(PropertyFlags));
  }

  void clear() {
    _flags = 0;
  }

  bool operator==(PropertyFlags f) const {
    return _flags == f._flags;
  }
  bool operator!=(PropertyFlags f) const {
    return _flags != f._flags;
  }

  /// \return true if this is not an invalid instance (i.e. the invalid flag
  ///   is not set).
  bool isValid() const {
    return !invalidFlags;
  }

  /// Return an instance of PropertyFlags initialized to the values that a
  /// new named property would have if it was added by "PutNamed".
  static PropertyFlags defaultNewNamedPropertyFlags() {
    PropertyFlags pf{};
    pf.enumerable = 1;
    pf.writable = 1;
    pf.configurable = 1;
    return pf;
  }

  /// Return an instance of PropertyFlags initialized to as writable,
  /// configurable and non-enumerable.
  static PropertyFlags nonEnumerablePropertyFlags() {
    PropertyFlags pf{};
    pf.enumerable = 0;
    pf.writable = 1;
    pf.configurable = 1;
    return pf;
  }

  /// Return the invalid instance.
  static PropertyFlags invalid() {
    PropertyFlags pf{};
    pf.invalidFlags = 1;
    return pf;
  }

  /// \p clear and \p set are masks for changing the property flags.
  /// `clear.flag1 = 1` clears the flag; `set.flag1 = 1` sets the flag.
  void changeFlags(PropertyFlags clear, PropertyFlags set) {
    _flags = (_flags & ~clear._flags) | set._flags;
  }
};

/// Index of a storage slot in a JavaScript object.
using SlotIndex = uint32_t;

static const SlotIndex INVALID_SLOT_INDEX = ~(SlotIndex)0;

/// Describes a single object property. This is the case class for
/// 'NamedPropertyDescriptor' and 'ComputedPropertyDescriptor', which have
/// exactly the same internal representation, but different semantics.
struct PropertyDescriptor {
  PropertyFlags flags;
  /// Storage slot of the property.
  SlotIndex slot;

  PropertyDescriptor(PropertyFlags flags, SlotIndex slot)
      : flags(flags), slot(slot) {}

  PropertyDescriptor() : slot(INVALID_SLOT_INDEX) {}
};

/// Describes a single "named" property. That is a property whose name is not a
/// valid array index string.
struct NamedPropertyDescriptor : public PropertyDescriptor {
  NamedPropertyDescriptor(PropertyFlags flags, SlotIndex slot)
      : PropertyDescriptor(flags, slot) {}

  NamedPropertyDescriptor() = default;
};

/// Describes a single object property when the property name was "computed". It
/// can be any property, including an array index.
struct ComputedPropertyDescriptor : public PropertyDescriptor {
  ComputedPropertyDescriptor(PropertyFlags flags, SlotIndex slot)
      : PropertyDescriptor(flags, slot) {}

  ComputedPropertyDescriptor() = default;

 private:
  friend class JSObject;

  /// An internal method to cast this as a reference to NamedPropertyDescriptor.
  NamedPropertyDescriptor &castToNamedPropertyDescriptorRef() {
    return *static_cast<NamedPropertyDescriptor *>(
        static_cast<PropertyDescriptor *>(this));
  }
};

/// @}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_PROPERTYDESCRIPTOR_H
