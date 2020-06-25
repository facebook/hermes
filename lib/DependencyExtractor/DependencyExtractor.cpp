/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/DependencyExtractor/DependencyExtractor.h"

#include "hermes/AST/RecursiveVisitor.h"

namespace hermes {

namespace {
using namespace hermes::ESTree;

/// Pair of (callee, kind) where calling the callee with a string argument
/// results in a dependency of the kind.
struct ResourceCallee {
  llvm::StringLiteral callee;
  DependencyKind kind;
  constexpr ResourceCallee(llvm::StringLiteral callee, DependencyKind kind)
      : callee(callee), kind(kind) {}
};

/// All the resource callees which can result in a dependency.
static constexpr ResourceCallee RESOURCE_CALLEES[] = {
    {"__jsResource", DependencyKind::Resource},
    {"__conditionallySplitJSResource", DependencyKind::Resource},
    {"JSResource", DependencyKind::Resource},
    {"ConditionallySplitJSResource", DependencyKind::Resource},
    {"PrefetchedJSResource", DependencyKind::PrefetchedResource},
    {"prefetchImport", DependencyKind::Resource}};

static constexpr uint32_t NUM_RESOURCE_CALLEES =
    sizeof(RESOURCE_CALLEES) / sizeof(RESOURCE_CALLEES[0]);

/// Visitor for extracting dependencies from a given node.
/// Reports errors on encountering nodes which should introduce dependencies
/// but are malformed.
class DependencyExtractor {
  /// Storage for the extracted dependencies.
  std::vector<Dependency> deps_{};

  /// SourceErrorManager for reporting errors, e.g. for invalid require() calls.
  SourceErrorManager &sm_;

  UniqueString *requireIdent_;
  UniqueString *jestIdent_;

  UniqueString *requireActualIdent_;
  UniqueString *requireMockIdent_;

  /// Store the RESOURCE_CALLEES uniqued for quick comparison in a loop.
  UniqueString *resourceIdents_[NUM_RESOURCE_CALLEES];

 public:
  DependencyExtractor(Context &astContext)
      : sm_(astContext.getSourceErrorManager()),
        requireIdent_(astContext.getStringTable().getString("require")),
        jestIdent_(astContext.getStringTable().getString("jest")),
        requireActualIdent_(
            astContext.getStringTable().getString("requireActual")),
        requireMockIdent_(
            astContext.getStringTable().getString("requireMock")) {
    for (uint32_t i = 0; i < NUM_RESOURCE_CALLEES; ++i) {
      llvm::StringLiteral callee = RESOURCE_CALLEES[i].callee;
      resourceIdents_[i] = astContext.getStringTable().getString(callee);
    }
  }

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

  void visit(ImportDeclarationNode *node) {
    auto *source = llvm::cast<StringLiteralNode>(node->_source);

    // Whether there was an import of a value.
    bool hasValue = false;

    // Whether there was an import of a type.
    bool hasType = false;

    // Check top-level import kind.
    if (node->_importKind->str() == "value") {
      hasValue = true;
    } else {
      hasType = true;
    }

    for (Node &child : node->_specifiers) {
      // Use dyn_cast because there can also be ImportDefaultSpecifier or
      // ImportNamespaceSpecifier here.
      if (auto *spec = llvm::dyn_cast<ImportSpecifierNode>(&child)) {
        if (spec->_importKind->str() == "value") {
          hasValue = true;
        } else {
          hasType = true;
        }
      }
    }

    if (hasValue) {
      addDependency(source->_value->str(), DependencyKind::ESM);
    }
    if (hasType) {
      addDependency(source->_value->str(), DependencyKind::Type);
    }

    visitESTreeChildren(*this, node);
  }

  void visit(ExportNamedDeclarationNode *node) {
    auto *source = llvm::cast_or_null<StringLiteralNode>(node->_source);
    if (source) {
      DependencyKind kind = node->_exportKind->str() == "value"
          ? DependencyKind::ESM
          : DependencyKind::Type;
      addDependency(source->_value->str(), kind);
    }
    visitESTreeChildren(*this, node);
  }

  void visit(ExportAllDeclarationNode *node) {
    auto *source = llvm::cast<StringLiteralNode>(node->_source);
    DependencyKind kind = node->_exportKind->str() == "value"
        ? DependencyKind::ESM
        : DependencyKind::Type;
    addDependency(source->_value->str(), kind);
    visitESTreeChildren(*this, node);
  }

  void visit(CallExpressionLikeNode *node) {
    // CallExpressionLike-based dependencies will need the first argument to be
    // a string indicating the source.
    auto *callee = getCallee(node);
    if (auto *import = llvm::dyn_cast<ImportNode>(callee)) {
      if (auto *name = needFirstStringArgument(node)) {
        addDependency(name->_value->str(), DependencyKind::Async);
      }
    } else if (auto *ident = llvm::dyn_cast<IdentifierNode>(callee)) {
      if (ident->_name == requireIdent_) {
        if (auto *name = needFirstStringArgument(node)) {
          addDependency(name->_value->str(), DependencyKind::Require);
        }
      } else {
        for (uint32_t i = 0; i < NUM_RESOURCE_CALLEES; ++i) {
          UniqueString *c = resourceIdents_[i];
          if (c == ident->_name) {
            if (auto *name = needFirstStringArgument(node)) {
              addDependency(name->_value->str(), RESOURCE_CALLEES[i].kind);
            }
            break;
          }
        }
      }
    } else if (auto *mem = llvm::dyn_cast<MemberExpressionNode>(callee)) {
      auto *obj = llvm::dyn_cast<IdentifierNode>(mem->_object);
      if (obj && !mem->_computed &&
          (obj->_name == requireIdent_ || obj->_name == jestIdent_)) {
        auto *prop = llvm::dyn_cast<IdentifierNode>(mem->_property);
        if (prop &&
            (prop->_name == requireActualIdent_ ||
             prop->_name == requireMockIdent_)) {
          if (auto *name = needFirstStringArgument(node)) {
            addDependency(name->_value->str(), DependencyKind::Require);
          }
        }
      }
    }
    visitESTreeChildren(*this, node);
  }

 private:
  /// Perform any postprocessing on \p name (e.g. removing "m#" at the start)
  /// and add the {name, kind} pair to the list of extracted dependencies.
  void addDependency(llvm::StringRef name, DependencyKind kind) {
    name.consume_front("m#");
    deps_.push_back(Dependency{name.str(), kind});
  }

  /// \return the first argument to \p node if it is a string literal.
  ///   Else, report an error to sm_ and return nullptr.
  StringLiteralNode *needFirstStringArgument(CallExpressionLikeNode *node) {
    NodeList &args = getArguments(node);
    if (args.empty()) {
      sm_.error(
          node->getSourceRange(),
          "dependency call needs a string literal argument");
      return nullptr;
    }
    auto *first = llvm::dyn_cast<StringLiteralNode>(&args.front());
    if (!first) {
      sm_.error(
          node->getSourceRange(),
          "dependency call needs a string literal argument");
      return nullptr;
    }
    return first;
  }
};

} // namespace

std::vector<Dependency> extractDependencies(Context &astContext, Node *node) {
  DependencyExtractor extract{astContext};
  extract.doIt(node);
  return std::move(extract.getDeps());
}

} // namespace hermes
