/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_KEYWORDS_H
#define HERMES_SEMA_KEYWORDS_H

namespace hermes {

class Context;
class UniqueString;

namespace sema {

class Keywords {
 public:
#define HERMES_KEYWORD(name, string) UniqueString *const ident##name;
#include "hermes/Sema/Keywords.def"

  explicit Keywords(Context &astContext);

 private:
  /// An unused field to handle the last "," in constructor init.
  int const dummy_;
};

} // namespace sema
} // namespace hermes

#endif
