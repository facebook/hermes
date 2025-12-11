/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_EXTENSIONS_CONTRIB_CONTRIBDUMMY_H
#define HERMES_EXTENSIONS_CONTRIB_CONTRIBDUMMY_H

#include <jsi/jsi.h>

namespace facebook {
namespace hermes {

/// ContribDummy extension - example community contribution.
/// Does nothing - serves as a template for contrib extensions.
///
/// \param runtime The JSI runtime to install into.
/// \param extensions The precompiled extensions object containing setup
///   functions.
void installContribDummy(jsi::Runtime &runtime, jsi::Object &extensions);

} // namespace hermes
} // namespace facebook

#endif // HERMES_EXTENSIONS_CONTRIB_CONTRIBDUMMY_H
