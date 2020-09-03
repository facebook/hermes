/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HERMESPARSER_HERMESPARSERJSBUILDER_H
#define HERMES_TOOLS_HERMESPARSER_HERMESPARSERJSBUILDER_H

#include "HermesParserJSLibrary.h"
#include "hermes/AST/ESTree.h"

namespace hermes {

JSReference buildProgram(ESTree::NodePtr rootNode, SourceErrorManager *sm);

} // namespace hermes

#endif
