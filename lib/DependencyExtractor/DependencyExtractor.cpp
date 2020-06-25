/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/DependencyExtractor/DependencyExtractor.h"

#include "hermes/AST/RecursiveVisitor.h"

namespace hermes {

using namespace hermes::ESTree;

/// Visitor for extracting dependencies from a given node.
/// Reports errors on encountering nodes which should introduce dependencies
/// but are malformed.
class DependencyExtractor {
  /// Storage for the extracted dependencies.
  std::vector<Dependency> deps_{};

  /// SourceErrorManager for reporting errors, e.g. for invalid require() calls.
  SourceErrorManager &sm_;

 public:
  DependencyExtractor(Context &astContext)
      : sm_(astContext.getSourceErrorManager()) {}

  std::vector<Dependency> &getDeps() {
    return deps_;
  }

  /// Perform the extraction on whole AST.
  void doIt(Node *rootNode) {
    visitESTreeNode(*this, rootNode);
  }

  /// Stub which catches all nodes we don't need to directly extract
  /// dependencies from.
  void visit(Node *node) {
    visitESTreeChildren(*this, node);
  }
};

std::vector<Dependency> extractDependencies(Context &astContext, Node *node) {
  DependencyExtractor extract{astContext};
  extract.doIt(node);
  return std::move(extract.getDeps());
}

} // namespace hermes
