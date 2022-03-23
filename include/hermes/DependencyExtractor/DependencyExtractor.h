/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_DEPENDENCYEXTRACTOR_H
#define HERMES_AST_DEPENDENCYEXTRACTOR_H

#include "hermes/AST/Context.h"
#include "hermes/AST/ESTree.h"

namespace hermes {

/// The kind of dependency, used to distinguish between the different ways to
/// pull an external module into the file.
enum class DependencyKind {
  // import {x} from 'source';
  ESM,
  // import type {x} from 'source';
  Type,
  // require('source');
  Require,
  // import('source');
  Async,
  // JSResource('source');
  Resource,
  // PrefetchedJSResource('source');
  PrefetchedResource,
  // graphql`fragment source` and others;
  GraphQL,
};

inline const char *dependencyKindStr(DependencyKind kind) {
  switch (kind) {
    case DependencyKind::ESM:
      return "ESM";
    case DependencyKind::Type:
      return "Type";
    case DependencyKind::Require:
      return "Require";
    case DependencyKind::Async:
      return "Async";
    case DependencyKind::Resource:
      return "Resource";
    case DependencyKind::PrefetchedResource:
      return "PrefetchedResource";
    case DependencyKind::GraphQL:
      return "GraphQL";
  }
  llvm_unreachable("No other kind of dependency");
}

/// Represents a single dependency from a source file.
struct Dependency {
  /// The target file name of the dependency.
  /// For example, `require('name')` is used to indicate a Require dependency on
  /// 'name'.
  std::string name;

  /// The kind of the dependency.
  DependencyKind kind;
};

/// Extract any dependencies in \p node and its children.
/// \return a list of Dependency indicating what the target is, as well as the
/// kind of dependency.
std::vector<Dependency> extractDependencies(
    Context &astContext,
    ESTree::Node *node);

} // namespace hermes

#endif
