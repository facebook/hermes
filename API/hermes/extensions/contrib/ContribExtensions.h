/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_EXTENSIONS_CONTRIB_CONTRIBEXTENSIONS_H
#define HERMES_EXTENSIONS_CONTRIB_CONTRIBEXTENSIONS_H

#include <jsi/jsi.h>

namespace facebook {
namespace hermes {

/// Install all contrib (community-contributed) extensions into the runtime.
///
/// \param runtime The JSI runtime to install into.
/// \param extensions The precompiled extensions object containing setup
///   functions.
void installContribExtensions(jsi::Runtime &runtime, jsi::Object &extensions);

} // namespace hermes
} // namespace facebook

#endif // HERMES_EXTENSIONS_CONTRIB_CONTRIBEXTENSIONS_H
