/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/Keywords.h"

#include "hermes/AST/Context.h"

namespace hermes::sema {

#define HERMES_KEYWORD(name, string) \
  ident##name(astContext.getIdentifier(string).getUnderlyingPointer()),

Keywords::Keywords(Context &astContext)
    :
#include "hermes/Sema/Keywords.def"
      dummy_(0) {
  (void)dummy_;
}

} // namespace hermes::sema
