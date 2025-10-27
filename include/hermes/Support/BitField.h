/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <limits>
#include <type_traits>

namespace hermes {

/// A template class to manage a bit field within a storage type.
///
/// This class provides utilities to extract, update, and validate values within
/// a specific bit range of a storage type. It ensures that the field fits
/// within the storage and provides a convenient alias for defining the next
/// field in the sequence.
///
/// Example usage of BitField:
/// \code
/// struct ExampleWithoutMacros {
///   uint32_t storage;
///   using F1 = BitField<uint32_t, uint8_t, 0, 6>;
///   using F2 = F1::NextField<uint16_t, 15>;
///
///   uint8_t getF1() const { return F1::extract(storage); }
///   void setF1(uint8_t value) { storage = F1::update(storage, value); }
///
///   uint16_t getF2() const { return F2::extract(storage); }
///   void setF2(uint16_t value) { storage = F2::update(storage, value); }
/// };
/// \endcode
///
/// Example usage of BitField with macros:
/// \code
/// struct Example {
///   HERMES_FIRST_BITFIELD(uint32_t, storage, uint8_t, F1, 6)
///   HERMES_NEXT_BITFIELD(Field1, storage, uint16_t, F2, 15)
/// };
/// \endcode
///
/// \param StorageType The unsigned integer type used for storage (e.g.,
/// uint8_t, uint32_t).
/// \param FieldType The type of the field being managed (e.g., bool, uint16_t).
/// \param BitPosition The starting bit position of the field within the storage
/// (0-based).
/// \param Width The number of bits occupied by the field.
template <
    typename StorageType,
    typename FieldType,
    unsigned BitPosition,
    unsigned Width>
class BitField {
  // Ensure StorageType is an unsigned integer type to avoid sign-related issues
  static_assert(
      std::is_unsigned_v<StorageType>,
      "StorageType must be an unsigned integer type");
  // Validate bit position and width constraints
  static_assert(
      BitPosition < sizeof(StorageType) * 8,
      "BitPosition is out of range");
  static_assert(Width > 0, "Width must be greater than 0");
  static_assert(
      BitPosition + Width <= sizeof(StorageType) * 8,
      "Field exceeds storage size");
  static_assert(
      Width <= sizeof(FieldType) * 8,
      "Field width exceeds field type size");

  /// Number of bits in the storage type
  static constexpr unsigned STORAGE_BITS = sizeof(StorageType) * 8;
  /// Precompute the mask for the field aligned to bit 0.
  static constexpr StorageType MASK =
      std::numeric_limits<StorageType>::max() >> (STORAGE_BITS - Width);
  /// Check if the field is a bitfield or just a field (it happens).
  static constexpr bool IS_BITFIELD = Width < STORAGE_BITS;

  using SignedStorageType = std::make_signed_t<StorageType>;

 public:
  /// Extracts the field value from the storage.
  ///
  /// Shifts the storage right by BitPosition and applies the mask to isolate
  /// the field bits, then casts the result to FieldType.
  ///
  /// \param storage The storage value containing the field.
  /// \return The extracted field value as FieldType.
  static FieldType extract(StorageType storage) {
    if constexpr (!IS_BITFIELD) {
      // If the field is the entire storage, no need to shift.
      return FieldType(storage);
    } else if constexpr (std::is_signed_v<FieldType>) {
      // Shift to the left first, for sign extension.
      return FieldType(
          SignedStorageType(storage << (STORAGE_BITS - Width - BitPosition)) >>
          (STORAGE_BITS - Width));
    } else {
      // For unsigned types, no sign extension is needed.
      return FieldType((storage >> BitPosition) & MASK);
    }
  }

  /// Updates the field in the storage with a new value and returns the updated
  /// storage.
  ///
  /// Clears the field's bits in the storage and sets them to the new value,
  /// ensuring only the specified bits are modified.
  ///
  /// \param storage The current storage value.
  /// \param value The new value to set in the field.
  /// \return The updated storage value.
  static StorageType update(StorageType storage, FieldType value) {
    if constexpr (IS_BITFIELD) {
      return (storage & ~(MASK << BitPosition)) |
          ((StorageType(value) & MASK) << BitPosition);
    } else {
      return StorageType(value);
    }
  }

  /// Checks if a value can fit within the field's width.
  ///
  /// Ensures that the value, when interpreted as StorageType, has no bits set
  /// beyond the field's width.
  ///
  /// \param value The value to validate.
  /// \return True if the value fits within Width bits, false otherwise.
  static bool isValid(FieldType value) {
    return extract(update(0, value)) == value;
  }

  /// Template alias for defining the next field in the storage.
  ///
  /// Automatically computes the next bit position as BitPosition + Width,
  /// reusing the same StorageType.
  ///
  /// \tparam NextFieldType The type of the next field.
  /// \tparam NextWidth The width of the next field.
  template <typename NextFieldType, unsigned NextWidth>
  using NextField =
      BitField<StorageType, NextFieldType, BitPosition + Width, NextWidth>;
};

#define HERMES_FIRST_BITFIELD(storage_type, storage, field_type, name, bits) \
  storage_type _##storage;                                                   \
  using name##Field = BitField<storage_type, field_type, 0, bits>;           \
  field_type get##name() const {                                             \
    return name##Field::extract(_##storage);                                 \
  }                                                                          \
  void set##name(field_type value) {                                         \
    _##storage = name##Field::update(_##storage, value);                     \
  }

#define HERMES_NEXT_BITFIELD(prev, storage, field_type, name, bits) \
  using name##Field = prev##Field::NextField<field_type, bits>;     \
  field_type get##name() const {                                    \
    return name##Field::extract(_##storage);                        \
  }                                                                 \
  void set##name(field_type value) {                                \
    _##storage = name##Field::update(_##storage, value);            \
  }

}; // namespace hermes
