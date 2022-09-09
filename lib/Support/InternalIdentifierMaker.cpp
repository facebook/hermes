/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/InternalIdentifierMaker.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {

Identifier InternalIdentifierMaker::next(llvh::StringRef hint) {
  llvh::SmallString<16> buf;
  llvh::raw_svector_ostream nameBuilder{buf};
  nameBuilder << "?anon_" << counter_++ << "_" << hint;
  return stringTable_.getIdentifier(nameBuilder.str());
}
} // namespace hermes
