/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_INTERNALIDENTIFIERMAKER_H
#define HERMES_SUPPORT_INTERNALIDENTIFIERMAKER_H

#include "hermes/Support/StringTable.h"
#include "llvh/ADT/StringRef.h"

namespace hermes {
/// Creates unique strings that the front-end may use to generate internal IDs.
/// These IDs are inaccessible from the JS source code.
class InternalIdentifierMaker {
  InternalIdentifierMaker(const InternalIdentifierMaker &) = delete;
  void operator=(const InternalIdentifierMaker &) = delete;

 public:
  explicit InternalIdentifierMaker(StringTable &stringTable)
      : stringTable_(stringTable) {}

  /// \return The next internal ID. \p hint is used to make the generated
  /// identifier's use clearer in the IR.
  Identifier next(llvh::StringRef hint);

 private:
  StringTable &stringTable_;
  size_t counter_{};
};

} // namespace hermes

#endif // HERMES_SUPPORT_INTERNALIDENTIFIERMAKER_H
