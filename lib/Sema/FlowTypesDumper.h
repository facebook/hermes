/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_FLOWTYPESDUMPER_H
#define HERMES_SEMA_FLOWTYPESDUMPER_H

#include "hermes/Sema/FlowContext.h"

namespace hermes {
namespace flow {

class FlowTypesDumper {
  struct TypesDenseMapInfo : public llvh::DenseMapInfo<const TypeInfo *> {
    // static inline T getEmptyKey();
    // static inline T getTombstoneKey();
    static unsigned getHashValue(const TypeInfo *val) {
      return val->hash();
    }
    static bool isEqual(const TypeInfo *LHS, const TypeInfo *RHS) {
      if (LHS == RHS)
        return true;
      auto const EMPTY = getEmptyKey();
      auto const TOMB = getTombstoneKey();
      if (LHS == EMPTY || LHS == TOMB || RHS == EMPTY || RHS == TOMB)
        return false;

      return LHS->equals(RHS);
    }
  };

  std::vector<const TypeInfo *> types_{};

  /// Map a type instance to a number.
  llvh::DenseMap<const TypeInfo *, size_t, TypesDenseMapInfo> typeNumber_{};

 public:
  /// \return the unique number associated with the type or 0 if it is a
  /// singleton.
  size_t getNumber(const TypeInfo *type);

  /// Print a reference to a type.
  void printTypeRef(llvh::raw_ostream &os, const TypeInfo *type);
  void printTypeRef(llvh::raw_ostream &os, const Type *type) {
    return printTypeRef(os, type->info);
  }

  /// Print a description of the type ending with a new line.
  void printTypeDescription(llvh::raw_ostream &os, const TypeInfo *type);

  /// Print the descriptions of all recorded types.
  void printAllNumberedTypes(llvh::raw_ostream &os);

  /// Print all existing types.
  void printAllTypes(llvh::raw_ostream &os, const FlowContext &flowTypes);
};

} // namespace flow
} // namespace hermes

#endif // HERMES_SEMA_FLOWTYPESDUMPER_H
