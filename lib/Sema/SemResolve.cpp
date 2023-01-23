/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Sema/SemResolve.h"

#include "SemanticResolver.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Support/PerfSection.h"

namespace hermes {
namespace sema {

class ASTPrinter {
  llvh::raw_ostream &os_;
  SemContextDumper &semDumper_;
  unsigned depth_ = 0;

 public:
  ASTPrinter(llvh::raw_ostream &os, SemContextDumper &semDumper)
      : os_(os), semDumper_(semDumper) {}

  void run(ESTree::Node *root) {
    ESTree::ESTreeVisit(*this, root);
    os_ << "\n";
  }

  bool shouldVisit(ESTree::Node *V) {
    return true;
  }

  void enter(ESTree::Node *V) {
    ++depth_;
    os_ << llvh::left_justify("", (depth_ - 1) * 4);
    os_ << V->getNodeName();
    printNodeType(V);
    printScopeRef(V);
    os_ << '\n';
  }
  void enter(ESTree::IdentifierNode *V) {
    ++depth_;
    os_ << llvh::left_justify("", (depth_ - 1) * 4);
    os_ << "Id '" << V->_name->str() << '\'';
    if (V->getDecl()) {
      os_ << " [";
      semDumper_.printDeclRef(os_, V->getDecl());
      os_ << ']';
    }
    printNodeType(V);
    os_ << '\n';
  }
  void leave(ESTree::Node *V) {
    --depth_;
  }

 private:
  void printScopeRef(ESTree::Node *n) {
    if (auto *sd = ESTree::getDecoration<ESTree::ScopeDecorationBase>(n)) {
      if (sd->getScope()) {
        os_ << " ";
        semDumper_.printScopeRef(os_, sd->getScope());
      }
    }
  }

  void printNodeType(ESTree::Node *n) {}
};

bool resolveAST(
    Context &astContext,
    SemContext &semCtx,
    ESTree::Node *root,
    const DeclarationFileListTy &ambientDecls) {
  PerfSection validation("Resolving JavaScript function AST");
  // Resolve the entire AST.
  SemanticResolver resolver{astContext, semCtx, ambientDecls, true};
  return resolver.run(root);
}

void semDump(llvh::raw_ostream &os, SemContext &semCtx, ESTree::Node *root) {
  SemContextDumper semDumper;
  semDumper.printSemContext(os, semCtx);
  os << '\n';
  ASTPrinter ap(os, semDumper);
  ap.run(root);
}

bool resolveASTForParser(
    Context &astContext,
    SemContext &semCtx,
    ESTree::Node *root) {
  SemanticResolver resolver{astContext, semCtx, {}, false};
  return resolver.run(root);
}

} // namespace sema
} // namespace hermes
