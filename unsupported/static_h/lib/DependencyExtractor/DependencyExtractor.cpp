/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/DependencyExtractor/DependencyExtractor.h"
#include "hermes/DependencyExtractor/GraphQLDependencyExtractor.h"

#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/Regex/Executor.h"
#include "hermes/Regex/Regex.h"
#include "hermes/Regex/RegexTraits.h"

namespace hermes {

namespace {
using namespace hermes::ESTree;

/// Pair of (callee, kind) where calling the callee with a string argument
/// results in a dependency of the kind.
struct ResourceCallee {
  llvh::StringLiteral callee;
  DependencyKind kind;
  constexpr ResourceCallee(llvh::StringLiteral callee, DependencyKind kind)
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

  UniqueString *graphqlIdent_;
  /// Bytecode for the regex that detects dependencies in GraphQL template
  /// literals.
  std::vector<uint8_t> graphqlQueryRegexBytecode_;

  /// Set to true when we have already found an registered a JSX dependency
  /// for this file.
  bool foundJSX_{false};

 public:
  DependencyExtractor(Context &astContext)
      : sm_(astContext.getSourceErrorManager()),
        requireIdent_(astContext.getStringTable().getString("require")),
        jestIdent_(astContext.getStringTable().getString("jest")),
        requireActualIdent_(
            astContext.getStringTable().getString("requireActual")),
        requireMockIdent_(astContext.getStringTable().getString("requireMock")),
        graphqlIdent_(astContext.getStringTable().getString("graphql")),
        graphqlQueryRegexBytecode_(graphql::getCompiledGraphQLRegex()) {
    for (uint32_t i = 0; i < NUM_RESOURCE_CALLEES; ++i) {
      llvh::StringLiteral callee = RESOURCE_CALLEES[i].callee;
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

  /// A dummy implementation of the AST stack overflow protocol.
  bool incRecursionDepth(Node *) const {
    return true;
  }
  /// A dummy implementation of the AST stack overflow protocol.
  void decRecursionDepth() const {}

  /// Stub which catches all nodes we don't need to directly extract
  /// dependencies from.
  void visit(Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ImportDeclarationNode *node) {
    auto *source = llvh::cast<StringLiteralNode>(node->_source);

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
      if (auto *spec = llvh::dyn_cast<ImportSpecifierNode>(&child)) {
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
    auto *source = llvh::cast_or_null<StringLiteralNode>(node->_source);
    if (source) {
      DependencyKind kind = node->_exportKind->str() == "value"
          ? DependencyKind::ESM
          : DependencyKind::Type;
      addDependency(source->_value->str(), kind);
    }
    visitESTreeChildren(*this, node);
  }

  void visit(ExportAllDeclarationNode *node) {
    auto *source = llvh::cast<StringLiteralNode>(node->_source);
    DependencyKind kind = node->_exportKind->str() == "value"
        ? DependencyKind::ESM
        : DependencyKind::Type;
    addDependency(source->_value->str(), kind);
    visitESTreeChildren(*this, node);
  }

  void visit(ImportExpressionNode *node) {
    if (StringLiteralNode *name =
            llvh::dyn_cast<StringLiteralNode>(node->_source)) {
      addDependency(name->_value->str(), DependencyKind::Async);
    }
  }

  void visit(CallExpressionLikeNode *node) {
    // CallExpressionLike-based dependencies will need the first argument to be
    // a string indicating the source.
    auto *callee = getCallee(node);
    if (auto *ident = llvh::dyn_cast<IdentifierNode>(callee)) {
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
    } else if (auto *mem = llvh::dyn_cast<MemberExpressionNode>(callee)) {
      auto *obj = llvh::dyn_cast<IdentifierNode>(mem->_object);
      if (obj && !mem->_computed &&
          (obj->_name == requireIdent_ || obj->_name == jestIdent_)) {
        auto *prop = llvh::dyn_cast<IdentifierNode>(mem->_property);
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

  void visit(TaggedTemplateExpressionNode *node) {
    if (auto *tagIdent = llvh::dyn_cast<IdentifierNode>(node->_tag)) {
      if (tagIdent->_name == graphqlIdent_) {
        NodeList &quasis =
            llvh::cast<TemplateLiteralNode>(node->_quasi)->_quasis;
        for (auto &it : quasis) {
          auto *quasi = llvh::cast<TemplateElementNode>(&it);
          registerGraphQLDependencies(quasi);
        }
      }
    }
    visitESTreeChildren(*this, node);
  }

  void visit(JSXElementNode *node) {
    registerJSXDependencies();
    visitESTreeChildren(*this, node);
  }

  void visit(JSXFragmentNode *node) {
    registerJSXDependencies();
    visitESTreeChildren(*this, node);
  }

 private:
  /// Perform any postprocessing on \p name (e.g. removing "m#" at the start)
  /// and add the {name, kind} pair to the list of extracted dependencies.
  void addDependency(llvh::StringRef name, DependencyKind kind) {
    name.consume_front("m#");
    switch (kind) {
      case DependencyKind::GraphQL:
        // GraphQL dependencies require a .graphql appended to them.
        deps_.push_back(Dependency{llvh::Twine(name, ".graphql").str(), kind});
        break;
      default:
        deps_.push_back(Dependency{name.str(), kind});
        break;
    }
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
    auto *first = llvh::dyn_cast<StringLiteralNode>(&args.front());
    if (!first) {
      sm_.error(
          node->getSourceRange(),
          "dependency call needs a string literal argument");
      return nullptr;
    }
    return first;
  }

  /// Run the graphqlQueryRegexBytecode_ against the contents of \p elem,
  /// and add any matches to the dependency list.
  void registerGraphQLDependencies(TemplateElementNode *elem) {
    if (!elem->_cooked)
      return;
    llvh::StringRef string = elem->_cooked->str();

    graphql::getGraphQLDependencies(
        string, graphqlQueryRegexBytecode_, [&](llvh::StringRef result) {
          addDependency(result, DependencyKind::GraphQL);
        });
  }

  /// If we have not seen JSX before, add the relevant JSX dependencies to the
  /// dependency list, and set foundJSX_ so we don't duplicate entries.
  void registerJSXDependencies() {
    if (foundJSX_)
      return;
    foundJSX_ = true;
    addDependency("react/jsx-runtime", DependencyKind::ESM);
    addDependency("react/jsx-dev-runtime", DependencyKind::ESM);
  }
};

} // namespace

std::vector<Dependency> extractDependencies(Context &astContext, Node *node) {
  DependencyExtractor extract{astContext};
  extract.doIt(node);
  return std::move(extract.getDeps());
}

} // namespace hermes
