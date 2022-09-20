/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_FRONTENDDEFS_JAVASCRIPT_DECLKIND_H_
#define HERMES_FRONTENDDEFS_JAVASCRIPT_DECLKIND_H_

namespace hermes {

enum class JavaScriptDeclKind : unsigned char { Const, Let, Var };

} // namespace hermes

#endif // HERMES_FRONTENDDEFS_JAVASCRIPT_DECLKIND_H_
